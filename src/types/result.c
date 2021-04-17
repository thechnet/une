/*
result.c - Une
Updated 2021-04-17
*/

#include "result.h"

#pragma region une_result_type_to_wcs
// FIXME: This function is technically not needed.
const wchar_t *une_result_type_to_wcs(une_result_type result_type)
{
  switch(result_type)
  {
    case UNE_RT_INT: return L"INT";
    case UNE_RT_FLT: return L"FLT";
    case UNE_RT_STR: return L"STR";
    case UNE_RT_LIST: return L"LIST";
    case UNE_RT_ERROR: return L"VOID";
    default: return L"Unknown Result Type!";
  }
}
#pragma endregion une_result_type_to_wcs

#pragma region une_result_is_true
une_int une_result_is_true(une_result result)
{
  switch(result.type)
  {
    case UNE_RT_INT: return result.value._int == 0 ? 0 : 1;
    case UNE_RT_FLT: return result.value._flt == 0.0 ? 0 : 1;
    case UNE_RT_STR: return wcslen(result.value._wcs) == 0 ? 0 : 1;
    case UNE_RT_LIST: return ((une_result*)result.value._vp)[0].value._int == 0 ? 0 : 1;
    default: WERR(L"une_result_is_true: Unhandled result type");
  }
}
#pragma endregion une_result_is_true

#pragma region une_results_are_equal
une_int une_results_are_equal(une_result left, une_result right)
{
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    return left.value._int == right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    return (float)left.value._int == right.value._flt;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    return left.value._flt == (float)right.value._int;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    return left.value._flt == right.value._flt;
  }
  // STR and STR
  else if(left.type == UNE_RT_STR && right.type == UNE_RT_STR)
  {
    return !wcscmp(left.value._wcs, right.value._wcs);
  }
  // LIST and LIST
  else if(left.type == UNE_RT_LIST && right.type == UNE_RT_LIST)
  {
    une_result *left_list = (une_result*)left.value._vp;
    une_result *right_list = (une_result*)right.value._vp;
    size_t left_size = left_list[0].value._int;
    size_t right_size = right_list[0].value._int;
    if(left_size != right_size)
    {
      return 0;
    }
    else
    {
      for(size_t i=1; i<=left_size; i++)
      {
        if(!une_results_are_equal(left_list[i], right_list[i])) return 0;
      }
    }
    return 1;
  }
  
  // Illegal Comparison
  return -1;
}
#pragma endregion une_results_are_equal

#pragma region une_result_to_wcs
wchar_t *une_result_to_wcs(une_result result)
{
  wchar_t *wcs = malloc(UNE_SIZE_MEDIUM *sizeof(*wcs));
  if(wcs == NULL) WERR(L"Out of memory.");
  size_t offset = swprintf(wcs, UNE_SIZE_MEDIUM,
                           UNE_COLOR_NEUTRAL L"%ls" UNE_COLOR_HINT L": " UNE_COLOR_SUCCESS,
                           une_result_type_to_wcs(result.type));
  switch(result.type)
  {
    case UNE_RT_INT:
      swprintf(wcs+offset, UNE_SIZE_MEDIUM, L"%lld", result.value._int);
      break;
    
    case UNE_RT_FLT:
      swprintf(wcs+offset, UNE_SIZE_MEDIUM, L"%.3f", result.value._flt);
      break;
    
    case UNE_RT_STR:
      swprintf(wcs+offset, UNE_SIZE_MEDIUM, UNE_COLOR_HINT L"\"" UNE_COLOR_SUCCESS L"%ls" UNE_COLOR_HINT L"\"", result.value._wcs);
      break;
    
    case UNE_RT_LIST: {
      une_result *list = (une_result*)result.value._vp;
      if(list == NULL) WERR(L"Undefined list pointer");
      size_t list_size = list[0].value._int;
      if(list_size == 0)
      {
        swprintf(wcs+offset, UNE_SIZE_MEDIUM, UNE_COLOR_NEUTRAL L"[]");
        break;
      }
      wchar_t *return_as_wcs = une_result_to_wcs(list[1]);
      offset += swprintf(wcs+offset, UNE_SIZE_MEDIUM,
                         UNE_COLOR_NEUTRAL L"[%ls",
                         return_as_wcs);
      free(return_as_wcs);
      for(size_t i=2; i<=list_size; i++)
      {
        return_as_wcs = une_result_to_wcs(list[i]);
        offset += swprintf(wcs+offset, UNE_SIZE_MEDIUM,
                           UNE_COLOR_NEUTRAL L", " UNE_COLOR_SUCCESS L"%ls",
                           return_as_wcs);
        free(return_as_wcs);
      }
      swprintf(wcs+offset, UNE_SIZE_MEDIUM, UNE_COLOR_NEUTRAL L"]");
      break; }
    
    case UNE_RT_ERROR:
      WERR(L"une_result_to_wcs: Illegal");
  }
  return wcs;
}
#pragma endregion une_result_to_wcs

#pragma region une_result_free
void une_result_free(une_result result)
{
  switch(result.type)
  {
    case UNE_RT_STR:
    case UNE_RT_ID:
      free(result.value._wcs);
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(L"Return: %d\n", result.type);
      #endif
      break;
    
    case UNE_RT_LIST: {
      une_result *list = (une_result*)result.value._vp;
      size_t list_size = list[0].value._int;
      /* Here we start at 0 because we _do_ want to free the UNE_RT_SIZE value. */
      for(size_t i=0; i<=list_size; i++)
      {
        une_result_free(list[i]);
      }
      free(list);
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(L"Return: %d\n", result.type);
      #endif
      break; }
    
    case UNE_RT_INT:
    case UNE_RT_FLT:
    case UNE_RT_ERROR:
    case UNE_RT_BREAK:
    case UNE_RT_CONTINUE:
    case UNE_RT_SIZE:
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(L"Return: %d\n", result.type);
      #endif
      break;
    
    default: WERR(L"Unhandled return value type in une_result_free()\n");
  }
}
#pragma endregion une_result_free

#pragma region une_result_copy
une_result une_result_copy(une_result src)
{
  une_result dest;
  dest.type = src.type;
  
  switch(src.type)
  {
    case UNE_RT_INT:
    case UNE_RT_ERROR: // FIXME:?
      dest.value._int = src.value._int;
      break;
    
    case UNE_RT_FLT:
      dest.value._flt = src.value._flt;
      break;
    
    case UNE_RT_STR: {
      wchar_t *string = malloc((wcslen(src.value._wcs+1)*sizeof(*string)));
      if(string == NULL) WERR(L"Out of memory.");
      wcscpy(string, src.value._wcs);
      dest.value._wcs = string;
      break; }
    
    case UNE_RT_LIST: {
      une_result *list = (une_result*)src.value._vp;
      size_t list_size = list[0].value._int;
      une_result *list_copy = malloc((list_size+1)*sizeof(*list_copy));
      if(list_copy == NULL) WERR(L"Out of memory.");
      /* Here we start at 0 because we _do_ want to copy the UNE_RT_ERROR value. */
      for(size_t i=0; i<=list_size; i++)
      {
        list_copy[i] = une_result_copy(list[i]);
      }
      dest.value._vp = (void*)list_copy;
      break; }
  }
  
  return dest;
}
#pragma endregion une_result_copy