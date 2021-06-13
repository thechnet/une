/*
builtin.c - Une
Modified 2021-06-09
*/

/* Header-specific includes. */
#include "builtin.h"

/* Implementation-specific includes. */
#include "tools.h"
#include <time.h>

/*
Get the une_builtin_type from a string containing a built-in function name.
*/
const une_builtin_type une_builtin_wcs_to_type(wchar_t *name)
{
  if (wcscmp(name, L"print") == 0)
    return UNE_BIF_PRINT;
  if (wcscmp(name, L"int") == 0)
    return UNE_BIF_TO_INT;
  if (wcscmp(name, L"float") == 0)
    return UNE_BIF_TO_FLT;
  if (wcscmp(name, L"str") == 0)
    return UNE_BIF_TO_STR;
  if (wcscmp(name, L"len") == 0)
    return UNE_BIF_GET_LEN;
  if (wcscmp(name, L"sleep") == 0)
    return UNE_BIF_SLEEP;
  return __UNE_BIF_none__;
}

/*
Get the number of parameters required by a built-in function.
*/
const une_int une_builtin_get_num_of_params(une_builtin_type type)
{
  switch (type) {
    case UNE_BIF_PRINT:
      return 1;
    case UNE_BIF_TO_INT:
      return 1;
    case UNE_BIF_TO_FLT:
      return 1;
    case UNE_BIF_TO_STR:
      return 1;
    case UNE_BIF_GET_LEN:
      return 1;
    case UNE_BIF_SLEEP:
      return 1;
    default:
      ERR(L"Unhandled type %d", type);
  }
}

/*
Print a text representation of a une_result.
*/
__une_builtin_fn(une_builtin_print, une_result result)
{
  /* Ensure une_result_type is data type. */
  if (!UNE_RESULT_TYPE_IS_DATA_TYPE(result.type)) {
    *error = UNE_ERROR_SET(UNE_ET_FUNCTION_UNEXPECTED_ARG_TYPE, pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Print text representation. */
  une_result_represent(result);
  
  return une_result_create(UNE_RT_VOID);
}

/*
Convert une_result to UNE_RT_INT une_result and return it.
*/
__une_builtin_fn(une_builtin_to_int, une_result result)
{
  /* Convert une_result. */
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
  
  /* Illegal input une_result_type. */
  *error = UNE_ERROR_SET(UNE_ET_FUNCTION_UNEXPECTED_ARG_TYPE, pos);
  return une_result_create(UNE_RT_ERROR);
}

/*
Convert une_result to UNE_RT_FLT une_result and return it.
*/
__une_builtin_fn(une_builtin_to_flt, une_result result)
{
  /* Convert une_result. */
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
  
  /* Illegal input une_result_type. */
  *error = UNE_ERROR_SET(UNE_ET_FUNCTION_UNEXPECTED_ARG_TYPE, pos);
  return une_result_create(UNE_RT_ERROR);
}

/*
Convert une_result to UNE_RT_STR une_result and return it.
*/
__une_builtin_fn(une_builtin_to_str, une_result result)
{
  /* Convert une_result. */
  switch (result.type) {
    case UNE_RT_INT: {
      wchar_t *out = une_malloc(UNE_SIZE_SMALL*sizeof(*out));
      swprintf(out, UNE_SIZE_SMALL, L"%lld", result.value._int);
      return (une_result){
        .type = UNE_RT_STR,
        .value._wcs = out
      };
    }
    case UNE_RT_FLT: {
      wchar_t *out = une_malloc(UNE_SIZE_SMALL*sizeof(*out));
      swprintf(out, UNE_SIZE_SMALL, L"%f", result.value._flt);
      return (une_result){
        .type = UNE_RT_STR,
        .value._wcs = out
      };
    }
    case UNE_RT_STR:
      return une_result_copy(result);
  }
  
  /* Illegal input une_result_type. */
  *error = UNE_ERROR_SET(UNE_ET_FUNCTION_UNEXPECTED_ARG_TYPE, pos);
  return une_result_create(UNE_RT_ERROR);
}

/*
Get length of une_result and return it.
*/
__une_builtin_fn(une_builtin_get_len, une_result result)
{
  /* Get length of une_result. */
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
  
  /* Illegal input une_result_type. */
  *error = UNE_ERROR_SET(UNE_ET_FUNCTION_UNEXPECTED_ARG_TYPE, pos);
  return une_result_create(UNE_RT_ERROR);
}

/*
Halt execution for a given amount of miliseconds.
*/
__une_builtin_fn(une_builtin_sleep, une_result result)
{
  /* Ensure input une_result_type is UNE_RT_INT. */
  if (result.type != UNE_RT_INT) {
    *error = UNE_ERROR_SET(UNE_ET_FUNCTION_UNEXPECTED_ARG_TYPE, pos);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Halt execution. */
  struct timespec ts = {
    .tv_sec = result.value._int / 1000,
    .tv_nsec = result.value._int % 1000 * 1000000
  };
  nanosleep(&ts, NULL);
  
  return une_result_create(UNE_RT_VOID);
}
