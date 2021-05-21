/*
builtin.c - Une
Modified 2021-05-21
*/

#include "builtin.h"

#pragma region une_builtin_wcs_to_type
const une_builtin_type une_builtin_wcs_to_type(wchar_t *name)
{
  if (wcscmp(name, L"print") == 0) return UNE_BIF_PRINT;
  if (wcscmp(name, L"int") == 0) return UNE_BIF_TO_INT;
  if (wcscmp(name, L"float") == 0) return UNE_BIF_TO_FLT;
  if (wcscmp(name, L"str") == 0) return UNE_BIF_TO_STR;
  if (wcscmp(name, L"len") == 0) return UNE_BIF_GET_LEN;
  return UNE_BIF_NONE;
}
#pragma endregion une_builtin_wcs_to_type

#pragma region une_builtin_get_num_of_params
const une_int une_builtin_get_num_of_params(une_builtin_type type)
{
  switch (type) {
    case UNE_BIF_PRINT: return 1;
    case UNE_BIF_TO_INT: return 1;
    case UNE_BIF_TO_FLT: return 1;
    case UNE_BIF_TO_STR: return 1;
    case UNE_BIF_GET_LEN: return 1;
    default: WERR(L"Unhandled type %d", type);
  }
}
#pragma endregion une_builtin_get_num_of_params

#pragma region une_builtin_print
une_result une_builtin_print(une_result result, une_context *context, une_position pos)
{
  switch (result.type) {
    case UNE_RT_VOID:
      context->error = UNE_ERROR_SET(UNE_ET_PRINT_VOID, pos);
      return une_result_create(UNE_RT_ERROR);
    case UNE_RT_INT:
      wprintf(L"%lld", result.value._int);
      break;
    case UNE_RT_FLT:
      wprintf(L"%f", result.value._flt);
      break;
    case UNE_RT_STR:
      wprintf(L"%ls", result.value._wcs);
      break;
    case UNE_RT_LIST: {
      UNE_UNPACK_RESULT_LIST(result, list, list_size);
      wprintf(L"[");
      if (list_size > 0) {
        wprintf(L"%ls", une_builtin_print(list[1], context, pos));
        for (size_t i=2; i<=list_size; i++) {
          wprintf(L", ");
          une_builtin_print(list[i], context, pos);
        }
      }
      wprintf(L"]");
      break;
    }
    default: WERR(L"Unhandled result type %d", result.type);
  }
  return une_result_create(UNE_RT_VOID);
}
#pragma endregion une_builtin_print

#pragma region une_builtin_to_int
une_result une_builtin_to_int(une_result result, une_context *context, une_position pos)
{
  switch (result.type) {
    case UNE_RT_INT:
      return une_result_copy(result);
    case UNE_RT_FLT:
      return (une_result){
        .type = UNE_RT_INT,
        .value._int = (une_int)result.value._flt
      };
    case UNE_RT_STR:
      return (une_result){
        .type = UNE_RT_INT,
        .value._int = wcs_to_une_int(result.value._wcs)
      };
  }
  context->error = UNE_ERROR_SETX(UNE_ET_CONVERSION, pos,
    _int=(int)result.type, _int=(int)UNE_RT_INT);
  return une_result_create(UNE_RT_ERROR);
}
#pragma endregion une_builtin_to_int

#pragma region une_builtin_to_flt
une_result une_builtin_to_flt(une_result result, une_context *context, une_position pos)
{
  switch (result.type) {
    case UNE_RT_INT:
      return (une_result){
        .type = UNE_RT_FLT,
        .value._flt = (une_flt)result.value._int
      };
    case UNE_RT_FLT:
      return une_result_copy(result);
    case UNE_RT_STR:
      return (une_result){
        .type = UNE_RT_FLT,
        .value._flt = wcs_to_une_flt(result.value._wcs)
      };
  }
  context->error = UNE_ERROR_SETX(UNE_ET_CONVERSION, pos,
    _int=(int)result.type, _int=(int)UNE_RT_FLT);
  return une_result_create(UNE_RT_ERROR);
}
#pragma endregion une_builtin_to_flt

#pragma region une_builtin_to_str
une_result une_builtin_to_str(une_result result, une_context *context, une_position pos)
{
  switch (result.type) {
    case UNE_RT_INT: {
      wchar_t *out = rmalloc(UNE_SIZE_SMALL*sizeof(*out));
      swprintf(out, UNE_SIZE_SMALL, L"%lld", result.value._int);
      return (une_result){
        .type = UNE_RT_STR,
        .value._wcs = out
      };
    }
    case UNE_RT_FLT: {
      wchar_t *out = rmalloc(UNE_SIZE_SMALL*sizeof(*out));
      swprintf(out, UNE_SIZE_SMALL, L"%f", result.value._flt);
      return (une_result){
        .type = UNE_RT_STR,
        .value._wcs = out
      };
    }
    case UNE_RT_STR:
      return une_result_copy(result);
  }
  context->error = UNE_ERROR_SETX(UNE_ET_CONVERSION, pos,
    _int=(int)result.type, _int=(int)UNE_RT_STR);
  return une_result_create(UNE_RT_ERROR);
}
#pragma endregion une_builtin_to_str

#pragma region une_builtin_get_len
une_result une_builtin_get_len(une_result result, une_context *context, une_position pos)
{
  switch (result.type) {
    case UNE_RT_STR:
      return (une_result){
        .type = UNE_RT_INT,
        .value._int = wcslen(result.value._wcs)
      };
    case UNE_RT_LIST: {
      UNE_UNPACK_RESULT_LIST(result, result_p, result_size);
      return (une_result){
        .type = UNE_RT_INT,
        .value._int = result_size
      };
    }
  }
  context->error = UNE_ERROR_SETX(UNE_ET_GETLEN, pos,
    _int=(int)result.type, _int=0);
  return une_result_create(UNE_RT_ERROR);
}
#pragma endregion une_builtin_get_len
