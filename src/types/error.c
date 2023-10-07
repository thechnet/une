/*
error.c - Une
Modified 2023-10-07
*/

/* Header-specific includes. */
#include "error.h"

/* Implementation-specific includes. */
#include "../lexer.h"

/*
Error message table.
*/
const wchar_t *une_error_message_table[] = {
  L"No error defined! (Internal Error)",
  L"Syntax.",
  L"Break outside loop.",
  L"Continue outside loop.",
  L"Symbol not defined.",
  L"Invalid index or slice.",
  L"Zero division.",
  L"Unreal number.",
  L"Wrong number of arguments.",
  L"File not found.",
  L"Encoding, conversion, or pattern error.",
  L"Type.",
  L"Assertion not met.",
  L"Misplaced 'any' or 'all'.",
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

size_t une_error_trace(une_error *error, une_lexer_state *ls, une_trace **out)
{
  une_context **lineage;
  size_t lineage_length = une_context_get_lineage(une_is->context, &lineage);
  
  size_t traces_length = lineage_length;
  une_trace *traces = malloc(traces_length*sizeof(*traces));
  verify(traces);
  for (size_t i=0; i<lineage_length-1; i++) {
    une_context *subject = lineage[lineage_length-i-2];
    char *file;
    if (subject->is_marker_context) {
      /* Use marker contexts to mark script() and eval() calls. */
      assert(subject->parent);
      file = subject->parent->has_callee ? subject->parent->callee_definition_file : subject->parent->creation_file;
    } else {
      file = subject->creation_file;
    }
    traces[i] = (une_trace){
      .file = file,
      .point = subject->creation_point,
      .function_label = subject->parent->callee_label,
      .function_file = subject->parent->callee_definition_file,
      .function_point = subject->parent->callee_definition_point
    };
  }
  
  traces[traces_length-1] = (une_trace){
    .file = une_is->context->has_callee ? une_is->context->callee_definition_file : une_is->context->creation_file,
    .point = error->pos,
    .function_label = une_is->context->callee_label,
    .function_file = une_is->context->callee_definition_file,
    .function_point = une_is->context->callee_definition_point
  };
  
  free(lineage);
  *out = traces;
  return traces_length;
}

void une_error_trace_print(une_trace trace)
{
  #define UNE_ERROR_TRACE_PRINT_PREFIX L"  "
  if (!trace.file)
    return;
  assert(une_file_exists(trace.file));
  FILE *f = fopen(trace.file, UNE_FOPEN_RFLAGS);
  assert(f);
  
  /* Skip to correct line. */
  size_t line_start = 0;
  size_t line = 1;
  wint_t wc = L' '; /* Initialize as whitespace in case no lines are skipped. */
  while (line < trace.point.line) {
    wc = fgetwc(f);
    if (wc == L'\n')
      line++;
    assert(wc != WEOF);
    line_start++;
  }
  
  /* Skip whitespace at start of line. */
  size_t text_start = line_start - 1 /* Account for first skipped newline. */;
  while (UNE_LEXER_WC_IS_WHITESPACE(wc) || UNE_LEXER_WC_IS_INVISIBLE(wc)) {
    wc = fgetwc(f);
    assert(wc != WEOF);
    text_start++;
  }
  
  /* Print code. */
  size_t text_end = text_start;
  fputws(UNE_ERROR_TRACE_PRINT_PREFIX, UNE_ERROR_STREAM);
  while (wc != L'\n' && wc != WEOF) {
    putwc((wchar_t)wc, UNE_ERROR_STREAM);
    wc = fgetwc(f);
    text_end++;
  }
  fclose(f);
  putwc(L'\n', UNE_ERROR_STREAM);
  
  /* Underline position. */
  fputws(UNE_COLOR_POSITION UNE_ERROR_TRACE_PRINT_PREFIX, UNE_ERROR_STREAM);
  for (size_t i=text_start; i<trace.point.start; i++)
    putwc(L' ', UNE_ERROR_STREAM);
  for (size_t i=trace.point.start; i<trace.point.end && i<=text_end; i++)
    putwc(L'~', UNE_ERROR_STREAM);
  if (trace.point.end > text_end + 1)
    fputws(L"\b+", UNE_ERROR_STREAM);
  fputws(UNE_COLOR_RESET L"\n", UNE_ERROR_STREAM);
  
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_DISPLAY_EXTENDED_ERROR)
  fwprintf(UNE_ERROR_STREAM, UNE_COLOR_HINT L"Want: line %zu, characters %zu-%zu\n", trace.point.line, trace.point.start, trace.point.end);
  fwprintf(UNE_ERROR_STREAM, L"Have: %zu (line), %zu (text), %zu (end)" UNE_COLOR_RESET L"\n", line_start, text_start, text_end);
  #endif
}

void une_error_display(une_error *error, une_lexer_state *ls)
{
  une_trace *traces = NULL;
  
  if (error->type == UNE_ET_FILE_NOT_FOUND && error->pos.end == 0)
    goto skip_traceback;
  
  size_t traces_length = une_error_trace(error, ls, &traces);
  
  for (size_t i=0; i<traces_length; i++) {
    char *file = traces[i].file ? traces[i].file : UNE_SOURCE_PLACEHOLDER;
    fwprintf(UNE_ERROR_STREAM, UNE_COLOR_TRACEBACK_LOCATION L"In file \"%hs\" at line %zu", file, traces[i].point.line);
    if (traces[i].function_file) {
      fputws(L", in", UNE_ERROR_STREAM);
      if (traces[i].function_label)
        fwprintf(UNE_ERROR_STREAM, L" %ls", traces[i].function_label);
      if (
        !traces[i].function_label
        #if defined(UNE_DEBUG) && defined(UNE_DEBUG_DISPLAY_EXTENDED_ERROR)
        || true
        #endif
      )
        fwprintf(UNE_ERROR_STREAM, L" (\"%hs\": %zu)", traces[i].function_file, traces[i].function_point.line);
    }
    fputws(L":" UNE_COLOR_RESET L"\n", UNE_ERROR_STREAM);
    une_error_trace_print(traces[i]);
  }
  free(traces);
  
  skip_traceback:
  
  fwprintf(UNE_ERROR_STREAM, UNE_COLOR_FAIL L"Error: %ls" UNE_COLOR_RESET L"\n", une_error_type_to_wcs(error->type));
  
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_DISPLAY_EXTENDED_ERROR)
  fwprintf(UNE_ERROR_STREAM, UNE_COLOR_HINT L"Source: %hs, %d" UNE_COLOR_RESET L"\n", error->meta_file, error->meta_line);
  #endif
}
