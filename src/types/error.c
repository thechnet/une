/*
error.c - Une
Updated 2021-05-10
*/

#include "error.h"

#pragma region une_error_value_to_wcs
wchar_t *une_error_value_to_wcs(une_error_type type, une_value *values)
{
  wchar_t *wcs = rmalloc(UNE_SIZE_MEDIUM*sizeof(*wcs));
  switch (type) {
    case UNE_ET_NO_ERROR:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"No error defined.");
      break;
    
    case UNE_ET_EXPECTED_TOKEN: {
      int offset = swprintf(wcs, UNE_SIZE_MEDIUM, L"Expected token: %ls",
                            une_token_type_to_wcs((une_token_type)values[0]._int));
      if (values[1]._wcs != NULL) {
        swprintf(wcs+offset, UNE_SIZE_MEDIUM, L" or %ls",
                 une_token_type_to_wcs((une_token_type)values[1]._int));
      }
      break; }
    
    case UNE_ET_UNEXPECTED_TOKEN:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Unexpected token: %ls",
               une_token_type_to_wcs((une_token_type)values[0]._int));
      break;
    
    case UNE_ET_UNEXPECTED_CHARACTER:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Unexpected character: '%lc'",
               (wchar_t)values[0]._int);
      break;
    
    case UNE_ET_INCOMPLETE_FLOAT:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Expected digit after '.'");
      break;
    
    case UNE_ET_ADD:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot add %ls to %ls",
               une_result_type_to_wcs((une_result_type)values[1]._int),
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;
    
    case UNE_ET_SUB:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot subtract %ls from %ls",
               une_result_type_to_wcs((une_result_type)values[1]._int),
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;
    
    case UNE_ET_MUL:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot multiply %ls by %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int),
               une_result_type_to_wcs((une_result_type)values[1]._int));
      break;
    
    case UNE_ET_DIV:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot divide %ls by %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int),
               une_result_type_to_wcs((une_result_type)values[1]._int));
      break;
    
    case UNE_ET_FDIV:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot floor divide %ls by %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int),
               une_result_type_to_wcs((une_result_type)values[1]._int));
      break;
    
    case UNE_ET_MOD:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot get modulus of %ls and %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int),
               une_result_type_to_wcs((une_result_type)values[1]._int));
      break;
    
    case UNE_ET_NEG:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot negate %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;
    
    case UNE_ET_POW:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot get %ls to the power of %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int),
               une_result_type_to_wcs((une_result_type)values[1]._int));
      break;
    
    case UNE_ET_ZERO_DIVISION:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot divide by zero");
      break;
    
    case UNE_ET_COMPARISON:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot compare %ls and %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int),
               une_result_type_to_wcs((une_result_type)values[1]._int));
      break;
    
    case UNE_ET_NOT_INDEXABLE:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot get index of %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;
    
    case UNE_ET_SET_NOT_INDEXABLE:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot set index of %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;
    
    case UNE_ET_NOT_INDEX_TYPE:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot use %ls as index",
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;
    
    case UNE_ET_INDEX_OUT_OF_RANGE:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Index %d is out of range", values[0]._int);
      break;
    
    case UNE_ET_SET:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot assign value to %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;
    
    case UNE_ET_GET:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Variable '%ls' does not exist", values[0]._wcs);
      break;
    
    case UNE_ET_FOR:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot use %ls as range value",
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;
    
    case UNE_ET_BREAK_OUTSIDE_LOOP:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot break outside loop");
      break;
    
    case UNE_ET_CONTINUE_OUTSIDE_LOOP:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot continue outside loop");
      break;
    
    case UNE_ET_UNTERMINATED_STRING:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Expected string termination");
      break;
    
    case UNE_ET_CANT_ESCAPE_CHAR:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot escape character: '%lc'",
               (wchar_t)values[0]._int);
      break;
    
    case UNE_ET_UNREAL_NUMBER:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Operation returns unreal number");
      break;
    
    case UNE_ET_SET_NO_ID:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot set index of literal %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;

    case UNE_ET_DEF:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Function '%ls' already defined", values[0]._wcs);
      break;

    case UNE_ET_CALL:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Function '%ls' not defined", values[0]._wcs);
      break;

    case UNE_ET_CALL_ARGS:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Expected %lld arguments, got %lld",
                 values[0]._int, values[1]._int);
      break;

    case UNE_ET_EXPECTED_RESULT_TYPE: {
      int offset = swprintf(wcs, UNE_SIZE_MEDIUM, L"Expected %ls",
                            une_result_type_to_wcs((une_result_type)values[0]._int));
      if (values[1]._wcs != NULL) {
        swprintf(wcs+offset, UNE_SIZE_MEDIUM, L" or %ls",
                 une_result_type_to_wcs((une_result_type)values[1]._int));
      }
      break; }

    case UNE_ET_PRINT_VOID:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot print VOID");
      break;
    
    case UNE_ET_CONVERSION:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot convert %ls to %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int),
               une_result_type_to_wcs((une_result_type)values[1]._int));
      break;
    
    case UNE_ET_GETLEN:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Cannot get length of %ls",
               une_result_type_to_wcs((une_result_type)values[0]._int));
      break;

    default:
      swprintf(wcs, UNE_SIZE_MEDIUM, L"Unknown error!");
      break;
  }
  return wcs;
}
#pragma endregion une_error_value_to_wcs

#pragma region une_error_display
void une_error_display(une_error error, wchar_t *text, wchar_t *name)
{
  int line = 1;
  size_t line_begin = 0;
  size_t line_end = 0;
  size_t pos_start = error.pos.start;
  size_t pos_end = error.pos.end;
  bool location_found = false;
  for (size_t i=0; i<wcslen(text)+1 /* Catch '\0' */ ; i++) {
    if (i == pos_start) location_found = true;
    if (location_found && (text[i] == L'\n' || text[i] == L'\0')) {
      line_end = i;
      break;
    }
    if (text[i] == L'\n') {
      line++;
      line_begin = i+1;
    }
  }
  wprintf(UNE_COLOR_NEUTRAL L"\33[1mFile %ls, Line %d (%hs @ %d):\33[0m"
          UNE_COLOR_NEUTRAL L"\n%.*ls\n",
          name, line, error.__file__, error.__line__,
          line_end-line_begin, text+line_begin);
  wprintf(L"\33[%dC%ls\33[1m" UNE_COLOR_FAIL,
          pos_start-line_begin, (pos_start-line_begin > 0) ? L"" : L"\33[D");
  if (pos_start >= pos_end) WERR(L"pos_start >= pos_end\n"); // DEBUG: For debugging only.
  for (int i=0; i<pos_end-pos_start; i++) {
    wprintf(L"~");
  }
  wchar_t *error_info_as_wcs = une_error_value_to_wcs(error.type, error.values);
  wprintf(UNE_COLOR_FAIL L"\33[1m\n%ls\n\n\33[0m" UNE_COLOR_HINT
          L"pos_start: %d\npos_end: %d\nline_begin: %d\nline_end: %d\33[0m\n",
          error_info_as_wcs,
          pos_start, pos_end, line_begin, line_end); // DEBUG: For debugging only.
  free(error_info_as_wcs);
}
#pragma endregion une_error_display

#pragma region une_error_free
void une_error_free(une_error error)
{
  switch (error.type) {
    case UNE_ET_NO_ERROR:
    case UNE_ET_EXPECTED_TOKEN:
    case UNE_ET_UNEXPECTED_TOKEN:
    case UNE_ET_UNEXPECTED_CHARACTER:
    case UNE_ET_INCOMPLETE_FLOAT:
    case UNE_ET_ADD:
    case UNE_ET_SUB:
    case UNE_ET_MUL:
    case UNE_ET_DIV:
    case UNE_ET_FDIV:
    case UNE_ET_MOD:
    case UNE_ET_NEG:
    case UNE_ET_POW:
    case UNE_ET_ZERO_DIVISION:
    case UNE_ET_COMPARISON:
    case UNE_ET_NOT_INDEXABLE:
    case UNE_ET_NOT_INDEX_TYPE:
    case UNE_ET_INDEX_OUT_OF_RANGE:
    case UNE_ET_SET:
    case UNE_ET_SET_NOT_INDEXABLE:
    case UNE_ET_UNREAL_NUMBER:
    case UNE_ET_SET_NO_ID:
    case UNE_ET_CALL_ARGS:
    case UNE_ET_EXPECTED_RESULT_TYPE:
    case UNE_ET_PRINT_VOID:
    case UNE_ET_CONVERSION:
    case UNE_ET_GETLEN:
    case UNE_ET_UNTERMINATED_STRING:
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Error: %d\n", __FILE__, __FUNCTION__, __LINE__, error.type);
      #endif
      break;
    
    case UNE_ET_GET:
    case UNE_ET_CALL:
      free(error.values[0]._wcs);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Error: %d\n", __FILE__, __FUNCTION__, __LINE__, error.type);
      #endif
      break;
    
    default: WERR(L"Unhandled error type %lld in une_error_free()!", error.type);
  }
}
#pragma endregion une_error_free

#pragma region une_error_copy
une_error une_error_copy(une_error src)
{
  une_error dest;
  dest.type = src.type;
  dest.pos = src.pos;
  dest.__line__ = src.__line__;
  dest.__file__ = src.__file__;

  switch (src.type) {
    case UNE_ET_NO_ERROR:
    case UNE_ET_EXPECTED_TOKEN:
    case UNE_ET_UNEXPECTED_TOKEN:
    case UNE_ET_UNEXPECTED_CHARACTER:
    case UNE_ET_INCOMPLETE_FLOAT:
    case UNE_ET_ADD:
    case UNE_ET_SUB:
    case UNE_ET_MUL:
    case UNE_ET_DIV:
    case UNE_ET_FDIV:
    case UNE_ET_MOD:
    case UNE_ET_NEG:
    case UNE_ET_POW:
    case UNE_ET_ZERO_DIVISION:
    case UNE_ET_COMPARISON:
    case UNE_ET_NOT_INDEXABLE:
    case UNE_ET_NOT_INDEX_TYPE:
    case UNE_ET_INDEX_OUT_OF_RANGE:
    case UNE_ET_SET:
    case UNE_ET_UNREAL_NUMBER:
    case UNE_ET_SET_NO_ID:
    case UNE_ET_CALL_ARGS:
    case UNE_ET_GETLEN:
      dest.values[0]._int = src.values[0]._int;
      dest.values[1]._int = src.values[1]._int;
      break;
    
    case UNE_ET_GET:
    case UNE_ET_CALL:
      dest.values[0]._wcs = wcs_dup(src.values[0]._wcs);
      break;
    
    default: WERR(L"Unhandled error type %lld in une_error_copy()!", src.type);
  }

  return dest;
}
#pragma endregion une_error_copy
