/*
error.c - Une
Modified 2021-07-15
*/

/* Header-specific includes. */
#include "error.h"

/* Implementation-specific includes. */
#include "../stream.h"

/*
Error message table.
*/
const wchar_t *une_error_message_table[] = {
  L"No error defined! (Internal Error)",
  L"Syntax error.",
  L"Break outside loop.",
  L"Continue outside loop.",
  L"Cannot assign to literal.",
  L"Symbol not defined.",
  L"Index out of range.",
  L"Zero division.",
  L"Unreal number.",
  L"Function already defined.",
  L"Wrong number of arguments.",
  L"File not found.",
  L"Encoding or conversion error.",
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
    .type = __UNE_ET_none__,
    .line = 0,
    .file = NULL
  };
}

/*
Get error message for error type.
*/
__une_static const wchar_t *une_error_type_to_wcs(une_error_type type)
{
  /* Ensure error type is within bounds. */
  if (!UNE_ERROR_TYPE_IS_VALID(type))
    type = __UNE_ET_max__;

  return une_error_message_table[type];
}

/*
Display error.
*/

UNE_ISTREAM_ARRAY_PULLER_VAL(__une_error_display_array_pull, wint_t, wchar_t, WEOF, true)
UNE_ISTREAM_ARRAY_ACCESS_VAL(__une_error_display_array_now, wint_t, wchar_t, WEOF, true)
UNE_ISTREAM_ARRAY_PEEKER_VAL(__une_error_display_array_peek, wint_t, wchar_t, WEOF, true)
UNE_ISTREAM_WFILE_PULLER(__une_error_display_wfile_pull)
UNE_ISTREAM_WFILE_ACCESS(__une_error_display_wfile_now)
UNE_ISTREAM_WFILE_PEEKER(__une_error_display_wfile_peek)
UNE_OSTREAM_PUSHER(__une_error_display_ctx_push, une_context*)

void une_error_display(une_error *error, une_lexer_state *ls, une_interpreter_state *is)
{
  /* Setup. */
  int line = 1;
  size_t line_begin = 0;
  size_t line_end = 0;
  size_t pos_start = error->pos.start;
  size_t pos_end = error->pos.end;
  une_istream text;
  wint_t (*pull)(une_istream*);
  wint_t (*peek)(une_istream*, ptrdiff_t);
  wint_t (*now)(une_istream*);
  const wchar_t *error_info_as_wcs; /* Predeclare to allow label jump. */

  if (error->type == UNE_ET_FILE_NOT_FOUND && error->pos.start == 0 && error->pos.end == 0)
    goto skip_preview;

  /* Get context traceback. */
  une_context **contexts = malloc(UNE_SIZE_EXPECTED_TRACEBACK_DEPTH*sizeof(*contexts));
  une_ostream s_contexts = une_ostream_create((void*)contexts, UNE_SIZE_EXPECTED_TRACEBACK_DEPTH, sizeof(*contexts), true);
  contexts = NULL; /* This pointer can turn stale after pushing. */
  void (*push)(une_ostream*, une_context*) = &__une_error_display_ctx_push;
  une_context *current_context = is->context;
  while (current_context != NULL) {
    push(&s_contexts, current_context);
    current_context = current_context->parent;
  }

  if (ls->read_from_file) {
    text = une_istream_wfile_create(ls->path);
    pull = &__une_error_display_wfile_pull;
    peek = &__une_error_display_wfile_peek;
    now = &__une_error_display_wfile_now;
  } else {
    text = une_istream_array_create((void*)ls->text, wcslen(ls->text));
    pull = &__une_error_display_array_pull;
    peek = &__une_error_display_array_peek;
    now = &__une_error_display_array_now;
  }

  /* Find error position in file. */
  assert(pos_start != pos_end);
  while (true) {
    if (pull(&text) == WEOF)
      break;
    if (text.index >= pos_start && (peek(&text, 1) == L'\n' || peek(&text, 1) == WEOF))
      break;
    if (now(&text) == L'\n')
      line_begin = text.index+1;
  }
  line_end = text.index;
  
  /* Print location of error. */
  wprintf(RESET BOLD L"File %ls, Line %d", is->context->name, line);
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_DISPLAY_EXTENDED_ERROR)
  wprintf(L" (%hs @ %d)", error->file, error->line);
  #endif
  wprintf(L":" RESET L"\n" RESET);
  contexts = (une_context**)s_contexts.array; /* Reobtain up-to-date pointer. */
  for (size_t i=s_contexts.index; i>0; i--)
    wprintf(BOLD L"(In %ls)" RESET L"\n", contexts[i-1]->name);
  free(contexts);
  
  /* Print text at location. */
  if (ls->read_from_file)
    une_istream_wfile_reset(&text);
  else
    une_istream_array_reset(&text);
  while (text.index+1 < (ptrdiff_t)line_begin)
    if (pull(&text) == WEOF)
      break;
  while (text.index < (ptrdiff_t)line_end) {
    if (pull(&text) == WEOF)
      break;
    wprintf(L"%c", now(&text));
  }
  wprintf(L"\n");

  /* Underline error position within line. */
  wprintf(UNE_COLOR_FAIL);
  for (int i=pos_start-line_begin; i>0; i--)
    putwc(L' ', stdout);
  for (int i=0; i<pos_end-pos_start; i++)
    putwc(L'~', stdout);
  putwc(L'\n', stdout);

  /* Close text stream. */
  if (ls->read_from_file)
    une_istream_wfile_free(text);
  
  skip_preview:

  /* Print error message. */
  error_info_as_wcs = une_error_type_to_wcs(error->type);
  wprintf(UNE_COLOR_FAIL L"%ls" RESET L"\n", error_info_as_wcs);

  /* Print more detailed information. */
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_DISPLAY_EXTENDED_ERROR)
  wprintf(L"\n");
  wprintf(L"pos_start: %d\n", pos_start);
  wprintf(L"pos_end: %d\n", pos_end);
  wprintf(L"line_begin: %d\n", line_begin);
  wprintf(L"line_end: %d\n", line_end);
  wprintf(L"\n");
  #endif
  
}
