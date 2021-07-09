/*
error.c - Une
Modified 2021-07-08
*/

/* Header-specific includes. */
#include "error.h"

/* Implementation-specific includes. */
#include "../stream.h"

/*
Error message table.
*/
const wchar_t *une_error_message_table[] = {
  L"No error defined!",
  L"Unexpected character.",
  L"Syntax error.",
  L"Cannot break outside loop.",
  L"Cannot continue outside loop.",
  L"Cannot assign to literal.",
  L"Variable not defined.",
  L"Index out of range.",
  L"Zero division.",
  L"Unreal number.",
  L"Function already defined.",
  L"Function not defined.",
  L"Wrong number of arguments.",
  L"File not found.",
  L"Encoding error.",
  L"Type conflict.",
  L"Unknown error!",
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

UNE_ISTREAM_ARRAY_PULLER_VAL(__une_error_display_array_pull, wint_t, wchar_t, WEOF, true);
UNE_ISTREAM_ARRAY_ACCESS_VAL(__une_error_display_array_now, wint_t, wchar_t, WEOF, true);
UNE_ISTREAM_WFILE_PULLER(__une_error_display_wfile_pull);
UNE_ISTREAM_WFILE_ACCESS(__une_error_display_wfile_now);

void une_error_display(une_error *error, une_lexer_state *ls, une_interpreter_state *is)
{
  /* Setup. */
  int line = 1;
  size_t line_begin = 0;
  size_t line_end = 0;
  size_t pos_start = error->pos.start;
  size_t pos_end = error->pos.end;
  bool location_found = false;
  une_istream text;
  wint_t (*pull)(une_istream*);
  wint_t (*now)(une_istream*);
  const wchar_t *error_info_as_wcs; /* Predeclare to allow label jump. */

  if (error->type == UNE_ET_FILE_NOT_FOUND && error->pos.start == 0 && error->pos.end == 0)
    goto skip_preview;

  if (ls->read_from_file) {
    text = une_istream_wfile_create(ls->path);
    pull = &__une_error_display_wfile_pull;
    now = &__une_error_display_wfile_now;
  } else {
    text = une_istream_array_create((void*)ls->text, wcslen(ls->text));
    pull = &__une_error_display_array_pull;
    now = &__une_error_display_array_now;
  }

  /* Find error position in file. */
  assert(pos_start != pos_end);
  while (pull(&text) != WEOF) {
    if (text.index == pos_start)
      location_found = true;
    if (location_found && now(&text) == L'\n')
      break;
    if (now(&text) == L'\n') {
      line++;
      line_begin = text.index;
    }
  }
  line_end = text.index;
  
  /* Print location of error. */
  wprintf(UNE_COLOR_NEUTRAL L"\33[1mFile %ls, Line %d", is->context->name, line);
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_DISPLAY_EXTENDED_ERROR)
  wprintf(L" (%hs @ %d)", error->file, error->line);
  #endif
  wprintf(L":\n" UNE_COLOR_NEUTRAL);

  /* Print text at location. */
  if (ls->read_from_file)
    une_istream_wfile_reset(&text);
  else
    une_istream_array_reset(&text);
  while (text.index+1 < (ptrdiff_t)line_begin)
    pull(&text);
  while (text.index+1 < (ptrdiff_t)line_end)
    wprintf(L"%c", pull(&text));
  wprintf(L"\n");

  /* Underline error position within line. */
  wprintf(L"\33[%dC%ls\33[1m" UNE_COLOR_FAIL, pos_start-line_begin-(line==1?0:1), (pos_start-line_begin > 0) ? L"" : L"\33[D");
  for (int i=0; i<pos_end-pos_start; i++)
    wprintf(L"~");
  wprintf(L"\n");

  /* Close text stream. */
  if (ls->read_from_file)
    une_istream_wfile_free(text);
  
  skip_preview:

  /* Print error message. */
  error_info_as_wcs = une_error_type_to_wcs(error->type);
  wprintf(UNE_COLOR_FAIL L"\33[1m%ls\33[0m\n", error_info_as_wcs);

  /* Print more detailed information. */
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_DISPLAY_EXTENDED_ERROR)
  wprintf(UNE_COLOR_HINT L"\npos_start: %d\npos_end: %d\nline_begin: %d\nline_end: %d\33[0m\n\n", pos_start, pos_end, line_begin, line_end);
  #endif
  
}
