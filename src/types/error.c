/*
error.c - Une
Modified 2023-02-09
*/

/* Header-specific includes. */
#include "error.h"

/* Implementation-specific includes. */
#include "../stream.h"
#include "../lexer.h"

#define ERROR_MESSAGE_TAB_WIDTH 8

/*
Error message table.
*/
const wchar_t *une_error_message_table[] = {
  L"No error defined! (Internal Error)",
  L"Syntax error.",
  L"Break outside loop.",
  L"Continue outside loop.",
  L"Symbol not defined.",
  L"Index out of range.",
  L"Zero division.",
  L"Unreal number.",
  L"Wrong number of arguments.",
  L"File not found.",
  L"Encoding, conversion, or pattern error.",
  L"Type error.",
  L"Unknown error! (Internal Error)",
};

/*
Initialize a une_error struct.
*/
une_error une_error_create(void)
{
  return (une_error){
    .pos = (une_position){
      .start = 0,
      .end = 0
    },
    .type = UNE_ET_none__,
    .meta_line = 0,
    .meta_file = NULL
  };
}

/*
Get error message for error type.
*/
une_static__ const wchar_t *une_error_type_to_wcs(une_error_type type)
{
  /* Ensure error type is within bounds. */
  if (!UNE_ERROR_TYPE_IS_VALID(type))
    type = UNE_ET_max__;

  return une_error_message_table[type];
}

/*
Display error.
*/

UNE_ISTREAM_ARRAY_PULLER_VAL(une_error_display_array_pull__, wint_t, wint_t, WEOF, true)
UNE_ISTREAM_ARRAY_ACCESS_VAL(une_error_display_array_now__, wint_t, wint_t, WEOF, true)
UNE_ISTREAM_ARRAY_PEEKER_VAL(une_error_display_array_peek__, wint_t, wint_t, WEOF, true)
UNE_ISTREAM_WFILE_PULLER(une_error_display_wfile_pull__)
UNE_ISTREAM_WFILE_ACCESS(une_error_display_wfile_now__)
UNE_ISTREAM_WFILE_PEEKER(une_error_display_wfile_peek__)
UNE_OSTREAM_PUSHER(une_error_display_ctx_push__, une_context*)

void une_error_display(une_error *error, une_lexer_state *ls, une_interpreter_state *is)
{
  /* Setup. */
  size_t invisible_characters = 0;
  size_t additional_characters = 0;
  une_istream text;
  wint_t (*pull)(une_istream*);
  wint_t (*peek)(une_istream*, ptrdiff_t);
  wint_t (*now)(une_istream*);
  void (*reset)(une_istream*);
  const wchar_t *error_info_as_wcs; /* Predeclare to allow label jump. */
  une_position error_line = { .start = 0, .end = 0 };

  /* Skip preview if not available. */
  if (error->type == UNE_ET_FILE_NOT_FOUND && error->pos.start == 0 && error->pos.end == 0)
    goto skip_preview;

  /* Get context traceback. */
  une_context **contexts = malloc(UNE_SIZE_EXPECTED_TRACEBACK_DEPTH*sizeof(*contexts));
  verify(contexts);
  une_ostream s_contexts = une_ostream_create((void*)contexts, UNE_SIZE_EXPECTED_TRACEBACK_DEPTH, sizeof(*contexts), true);
  contexts = NULL; /* This pointer can turn stale after pushing. */
  void (*push)(une_ostream*, une_context*) = &une_error_display_ctx_push__;
  une_context *current_context = is->context;
  push(&s_contexts, NULL);
  while (current_context != NULL) {
    push(&s_contexts, current_context);
    current_context = current_context->parent;
  }
  contexts = (une_context**)s_contexts.array; /* Reobtain up-to-date pointer. */
  contexts[0] = contexts[(s_contexts.index)--];
  contexts = NULL;

  /* Setup stream. */
  if (ls->read_from_file) {
    text = une_istream_wfile_create(ls->path);
    pull = &une_error_display_wfile_pull__;
    peek = &une_error_display_wfile_peek__;
    now = &une_error_display_wfile_now__;
    reset = &une_istream_wfile_reset;
  } else {
    text = une_istream_array_create((void*)ls->text, wcslen(ls->text));
    pull = &une_error_display_array_pull__;
    peek = &une_error_display_array_peek__;
    now = &une_error_display_array_now__;
    reset = &une_istream_array_reset;
  }

  /* Print traceback. */
  contexts = (une_context**)s_contexts.array; /* Reobtain up-to-date pointer. */
  for (int i=0; i<=s_contexts.index; i++) {
    
    /* Get stored information. */
    une_function *function;
    if (contexts[i]->function < 0)
      function = NULL;
    else
      function = (is->functions)+((contexts[i])->function);
    char *name;
    une_position position;
    bool main_context = false;
    if (function == NULL) {
      main_context = true;
      if (ls->read_from_file)
        name = ls->path;
      else
        name = UNE_COMMAND_LINE_NAME;
      position = error->pos;
    } else {
      name = function->definition_file;
      position = function->definition_point;
    }
    
    /* Compute line number. */
    int line = 1;
    size_t line_begin = 0, line_end = 0;
    assert(position.start != position.end);
    reset(&text);
    while (true) {
      if (pull(&text) == WEOF)
        break;
      if (main_context && text.index >= (ptrdiff_t)position.start && text.index < (ptrdiff_t)position.end) {
        if (UNE_LEXER_WC_IS_INVISIBLE(now(&text)))
          invisible_characters++;
        if (now(&text) == L'\t')
          additional_characters += ERROR_MESSAGE_TAB_WIDTH-1;
      }
      if (text.index >= (ptrdiff_t)position.start && (peek(&text, 1) == L'\n' || peek(&text, 1) == WEOF))
        break;
      if (now(&text) == L'\n') {
        line_begin = (size_t)text.index+1;
        if (main_context)
          invisible_characters = 0;
        line++;
      }
    }
    line_end = (size_t)text.index;
    
    /* Print position. */
    if (main_context) {
      error_line.start = line_begin;
      error_line.end = line_end;
      wprintf(UNE_COLOR_FAIL L"Error in file \"%hs\" at line %d", name, line);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_DISPLAY_EXTENDED_ERROR)
      wprintf(UNE_COLOR_HINT L"\n(%hs @ %d)\n", error->meta_file, error->meta_line);
      wprintf(L"(invisible_characters: %d)\n", invisible_characters);
      wprintf(L"(additional_characters: %d)\n", additional_characters);
      wprintf(L"(error_line.start/end: %d, %d)", error_line.start, error_line.end);
      #endif
    } else {
      wprintf(L",\nIn function {\"%hs\":%d}", name, line);
    }
    
    /* Print more detailed information. */
    #if defined(UNE_DEBUG) && defined(UNE_DEBUG_DISPLAY_EXTENDED_ERROR)
    wprintf(UNE_COLOR_HINT L"\n(position.start/end:   %d, %d)\n", position.start, position.end);
    wprintf(L"(line_begin/end:       %d, %d)" UNE_COLOR_FAIL, line_begin, line_end);
    #endif
  }
  wprintf(L":\n" RESET);
  
  free(s_contexts.array);
  
  /* Print text at location. */
  if (ls->read_from_file)
    une_istream_wfile_reset(&text);
  else
    une_istream_array_reset(&text);
  while (text.index+1 < (ptrdiff_t)error_line.start)
    if (pull(&text) == WEOF)
      break;
  while (text.index < (ptrdiff_t)error_line.end) {
    if (pull(&text) == WEOF)
      break;
    putwc((wchar_t)now(&text), stdout);
  }
  putwc(L'\n', stdout);

  /* Underline error position within line. */
  wprintf(UNE_COLOR_FAIL);
  for (size_t i=error->pos.start-error_line.start-invisible_characters+additional_characters; i>0; i--)
    putwc(L' ', stdout);
  for (size_t i=0; i<error->pos.end-error->pos.start; i++)
    putwc(L'~', stdout);
  putwc(L'\n', stdout);

  /* Close text stream. */
  if (ls->read_from_file)
    une_istream_wfile_free(text);
  
  skip_preview:

  /* Print error message. */
  error_info_as_wcs = une_error_type_to_wcs(error->type);
  wprintf(UNE_COLOR_FAIL L"%ls" RESET L"\n", error_info_as_wcs);
}
