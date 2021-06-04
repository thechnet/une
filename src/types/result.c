/*
result.c - Une
Updated 2021-06-04
*/

#include "result.h"

#pragma region Result Name Table
const wchar_t *une_result_table[] = {
  L"VOID",
  L"ERROR",
  L"INT",
  L"FLT",
  L"STR",
  L"LIST",
  L"ID",
  L"CONTINUE",
  L"BREAK",
  L"SIZE"
};
#pragma endregion Result Name Table

#pragma region une_result_type_to_wcs
/* DOC:
Returns a text representation of a une_result type.
*/
const wchar_t *une_result_type_to_wcs(une_result_type type)
{
  if (type <= 0 || type >= __UNE_RT_max__) WERR(L"Result type out of bounds: %d", type);
  return une_result_table[type-1];
}
#pragma endregion une_result_type_to_wcs

#pragma region une_result_create
une_result une_result_create(une_result_type type)
{
  return (une_result){
    .type = type,
    .value._vp = NULL,
  };
}
#pragma endregion une_result_create

#pragma region une_result_list_create
une_result *une_result_list_create(size_t items)
{
  une_result *list = rmalloc((items+1)*sizeof(*list));
  list[0] = (une_result){.type = UNE_RT_SIZE, .value._int = items};
  return list;
}
#pragma endregion une_result_list_create

#pragma region une_result_is_true
/* DOC:
Returns 1 if the input is considered 'true', otherwise 0.
*/
une_int une_result_is_true(une_result result)
{
  switch (result.type) {
    case UNE_RT_INT: return result.value._int == 0 ? 0 : 1;
    case UNE_RT_FLT: return result.value._flt == 0.0 ? 0 : 1;
    case UNE_RT_STR: return wcslen(result.value._wcs) == 0 ? 0 : 1;
    case UNE_RT_LIST:
      return ((une_result*)result.value._vp)[0].value._int == 0 ? 0 : 1;
    default: WERR(L"Unhandled result type");
  }
}
#pragma endregion une_result_is_true

#pragma region une_results_are_equal
/* DOC:
Returns 1 if the inputs are considered equal, otherwise 0.
*/
une_int une_results_are_equal(une_result left, une_result right)
{
  // INT and FLT
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    return left.value._int == right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    return (une_flt)left.value._int == right.value._flt;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    return left.value._flt == (une_flt)right.value._int;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    return left.value._flt == right.value._flt;
  }
  // STR and STR
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR) {
    return !wcscmp(left.value._wcs, right.value._wcs);
  }
  // LIST and LIST
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    une_result *left_list = (une_result*)left.value._vp;
    une_result *right_list = (une_result*)right.value._vp;
    size_t left_size = left_list[0].value._int;
    size_t right_size = right_list[0].value._int;
    if (left_size != right_size) {
      return 0;
    } else {
      UNE_FOR_NODE_LIST_ITEM(i, left_size) {
        if (!une_results_are_equal(left_list[i], right_list[i])) return 0;
      }
    }
    return 1;
  }
  
  // Illegal Comparison
  return -1;
}
#pragma endregion une_results_are_equal

#pragma region une_result_to_wcs
/* DOC:
Returns a text representation of a une_result.
*/
wchar_t *une_result_to_wcs(une_result result)
{
  wchar_t *wcs = rmalloc(UNE_SIZE_BIG * sizeof(*wcs));
  
  size_t offset = swprintf(wcs, UNE_SIZE_BIG, UNE_COLOR_NEUTRAL L"%ls",
                           une_result_type_to_wcs(result.type));
  
  if (result.type == UNE_RT_VOID || result.type == UNE_RT_ERROR) {
    return wcs;
  }
  
  offset += swprintf(wcs+offset, UNE_SIZE_BIG,
                     UNE_COLOR_HINT L": " UNE_COLOR_SUCCESS);

  switch (result.type) {
    case UNE_RT_INT:
      swprintf(wcs+offset, UNE_SIZE_BIG, L"%lld\33[0m", result.value._int);
      break;
    
    case UNE_RT_FLT:
      swprintf(wcs+offset, UNE_SIZE_BIG, L"%.3f\33[0m", result.value._flt);
      break;
    
    case UNE_RT_STR:
      swprintf(wcs+offset, UNE_SIZE_BIG, UNE_COLOR_HINT L"\"" UNE_COLOR_SUCCESS L"%ls" UNE_COLOR_HINT L"\"\33[0m", result.value._wcs);
      break;
    
    case UNE_RT_LIST: {
      une_result *list = (une_result*)result.value._vp;
      if (list == NULL) WERR(L"Undefined list pointer");
      size_t list_size = list[0].value._int;
      if (list_size == 0) {
        swprintf(wcs+offset, UNE_SIZE_BIG, UNE_COLOR_NEUTRAL L"[]\33[0m");
        break;
      }
      wchar_t *return_as_wcs = une_result_to_wcs(list[1]);
      offset += swprintf(wcs+offset, UNE_SIZE_BIG,
                         UNE_COLOR_NEUTRAL L"[%ls",
                         return_as_wcs);
      free(return_as_wcs);
      for (size_t i=2; i<=list_size; i++) {
        return_as_wcs = une_result_to_wcs(list[i]);
        offset += swprintf(wcs+offset, UNE_SIZE_BIG,
                           UNE_COLOR_NEUTRAL L", " UNE_COLOR_SUCCESS L"%ls",
                           return_as_wcs);
        free(return_as_wcs);
      }
      swprintf(wcs+offset, UNE_SIZE_BIG, UNE_COLOR_NEUTRAL L"]\33[0m");
      break; }
    
    default:
      WERR(L"Illegal (%d)", result.type);
  }
  return wcs;
}
#pragma endregion une_result_to_wcs

#pragma region une_result_free
/* DOC:
Frees a une_result object and all its contents.
*/
void une_result_free(une_result result)
{
  if (
    result.type <= __UNE_RT_none__ ||
    result.type >= __UNE_RT_max__
  ) {
    WERR(L"result.type=%d", (int)result.type)
  }
  
  switch (result.type) {
    case UNE_RT_STR:
    case UNE_RT_ID:
      free(result.value._wcs);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Return: %d\n", __FILE__, __FUNCTION__, __LINE__, result.type);
      #endif
      break;
    
    case UNE_RT_LIST: {
      une_result *list = (une_result*)result.value._vp;
      size_t list_size = list[0].value._int;
      /* Here we start at 0 because we _do_ want to free the UNE_RT_SIZE value. */
      for (size_t i=0; i<=list_size; i++) {
        une_result_free(list[i]);
      }
      free(list);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Return: %d\n", __FILE__, __FUNCTION__, __LINE__, result.type);
      #endif
      break; }
    
    #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
    default:
      wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Return: %d\n", __FILE__, __FUNCTION__, __LINE__, result.type);
      break;
    #endif
  }
}
#pragma endregion une_result_free

#pragma region une_result_copy
/* DOC:
Duplicates a une_result object with all its contents.
*/
une_result une_result_copy(une_result src)
{
  une_result dest = une_result_create(UNE_RT_VOID);
  dest.type = src.type;
  
  switch (src.type) {
    case UNE_RT_CONTINUE:
    case UNE_RT_BREAK:
    case UNE_RT_VOID:
      break;

    case UNE_RT_INT:
    case UNE_RT_ERROR:
    case UNE_RT_SIZE:
    case UNE_RT_FLT:
      dest.value = src.value;
      break;
    
    case UNE_RT_ID:
    case UNE_RT_STR:
      dest.value._wcs = wcs_dup(src.value._wcs);
      break;
    
    case UNE_RT_LIST: {
      une_result *list = (une_result*)src.value._vp;
      size_t list_size = list[0].value._int;
      une_result *list_copy = rmalloc((list_size+1)*sizeof(*list_copy));
      /* Here we start at 0 because we _do_ want to copy the UNE_RT_SIZE value. */
      for (size_t i=0; i<=list_size; i++) {
        list_copy[i] = une_result_copy(list[i]);
      }
      dest.value._vp = (void*)list_copy;
      break; }
    
    default:
      WERR(L"Unhandled result type %lld", src.type);
  }
  
  return dest;
}
#pragma endregion une_result_copy

#pragma region une_result_lists_add
une_result une_result_lists_add(une_result left, une_result right)
{
  UNE_UNPACK_RESULT_LIST(left, left_list, left_size);
  UNE_UNPACK_RESULT_LIST(right, right_list, right_size);
  
  une_result *new = une_result_list_create(left_size+right_size);

  UNE_FOR_NODE_LIST_ITEM(i, left_size) {
    new[i] = une_result_copy(left_list[i]);
  }
  UNE_FOR_NODE_LIST_ITEM(i, right_size) {
    new[left_size+i] = une_result_copy(right_list[i]);
  }

  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)new
  };
}
#pragma endregion une_result_lists_add

#pragma region une_result_list_mul
une_result une_result_list_mul(une_result list, une_int count)
{
  UNE_UNPACK_RESULT_LIST(list, list_p, list_size);

  une_result *new = rmalloc((count*list_size+1)*sizeof(*new));

  new[0] = (une_result){
    .type = UNE_RT_SIZE,
    .value._int = count*list_size
  };

  for (size_t i=0; i<count; i++) {
    UNE_FOR_RESULT_LIST_ITEM(j, list_size) {
      new[i*list_size+j] = une_result_copy(list_p[j]);
    }
  }

  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)new
  };
}
#pragma endregion une_result_list_mul

#pragma region une_result_strs_add
une_result une_result_strs_add(une_result left, une_result right)
{
  size_t left_size = wcslen(left.value._wcs);
  size_t right_size = wcslen(right.value._wcs);

  wchar_t *new = rmalloc((left_size+right_size+1)*sizeof(*new));

  wmemcpy(new, left.value._wcs, left_size);
  wmemcpy(new+left_size, right.value._wcs, right_size);
  new[left_size+right_size] = L'\0';

  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = new
  };
}
#pragma endregion une_result_strs_add

#pragma region une_result_str_mul
une_result une_result_str_mul(une_result str, une_int count)
{
  size_t str_size = wcslen(str.value._wcs);

  wchar_t *new = rmalloc((count*str_size+1)*sizeof(*new));

  for (size_t i=0; i<count; i++) {
    wmemcpy(new+i*str_size, str.value._wcs, str_size);
  }

  new[count*str_size] = L'\0';

  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = new
  };
}
#pragma endregion une_result_str_mul