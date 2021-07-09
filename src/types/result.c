/*
result.c - Une
Modified 2021-07-09
*/

/* Header-specific includes. */
#include "result.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "node.h"

/*
Result name table.
*/
const wchar_t *une_result_table[] = {
  L"VOID",
  L"ERROR",
  L"CONTINUE",
  L"BREAK",
  L"SIZE"
  L"INT",
  L"FLT",
  L"STR",
  L"LIST",
};

/*
Initialize a une_result.
*/
une_result une_result_create(une_result_type type)
{
  return (une_result){
    .type = type,
    .value._vp = NULL,
  };
}

/*
Return a duplicate une_result.
*/
une_result une_result_copy(une_result src)
{
  /* Ensure valid une_result_type. */
  UNE_VERIFY_RESULT_TYPE(src.type);
  
  /* Create destination une_result. */
  une_result dest = une_result_create(src.type);
  
  /* Populate destination. */
  switch (src.type) {
    
    /* Heap data. */
    case UNE_RT_STR:
      dest.value._wcs = wcsdup(src.value._wcs);
      break;
    
    /* List. */
    case UNE_RT_LIST: {
      UNE_UNPACK_RESULT_LIST(src, list, list_size);
      une_result *list_copy = une_result_list_create(list_size);
      UNE_FOR_RESULT_LIST_ITEM(i, list_size)
        list_copy[i] = une_result_copy(list[i]);
      dest.value._vp = (void*)list_copy;
      break;
    }
    
    /* Stack data. */
    default:
      dest.value = src.value;
      break;
    
  }
  
  return dest;
}

/*
Free all members of a une_result.
*/
void une_result_free(une_result result)
{
  /* Ensure valid une_result_type. */
  UNE_VERIFY_RESULT_TYPE(result.type);
  
  /* Free members. */
  switch (result.type) {
    
    /* Heap data. */
    case UNE_RT_STR:
      LOGFREE(L"une_result", une_result_type_to_wcs(result.type), result.type);
      free(result.value._wcs);
      break;
    
    /* List. */
    case UNE_RT_LIST: {
      LOGFREE(L"une_result", une_result_type_to_wcs(result.type), result.type);
      UNE_UNPACK_RESULT_LIST(result, list, list_size);
      UNE_FOR_RESULT_LIST_INDEX(i, list_size)
        une_result_free(list[i]);
      free(list);
      break;
    }
  
  }
}

/*
Return a list of uninitialized une_results.
*/
une_result *une_result_list_create(size_t items)
{
  une_result *list = malloc((items+1)*sizeof(*list));
  list[0] = (une_result){
    .type = UNE_RT_SIZE,
    .value._int = items
  };
  return list;
}

/*
Return a text representation of a une_result_type.
*/
const wchar_t *une_result_type_to_wcs(une_result_type type)
{
  /* Ensure valid une_result_type. */
  UNE_VERIFY_RESULT_TYPE(type);
  
  return une_result_table[type-1];
}

/*
Print a text representation of a une_result.
une_result does not have a *_to_wcs function because these functions can end in a buffer overflow.
As une_result needs to be representable in release builds, it instead has a *_represent function.
*/
void une_result_represent(une_result result)
{
  #ifndef UNE_DEBUG
  if (!UNE_RESULT_TYPE_IS_DATA_TYPE(result.type))
    fail(L"Cannot represent non-data result.");
  #endif

  switch (result.type) {
    
    case UNE_RT_INT:
      wprintf(UNE_COLOR_NEUTRAL L"%lld", result.value._int);
      break;
    
    case UNE_RT_FLT:
      wprintf(UNE_COLOR_NEUTRAL L"%.3f", result.value._flt);
      break;
    
    case UNE_RT_STR:
      wprintf(UNE_COLOR_NEUTRAL L"%ls", result.value._wcs);
      break;
    
    case UNE_RT_LIST:
      wprintf(UNE_COLOR_NEUTRAL L"[", stdout);
      UNE_UNPACK_RESULT_LIST(result, list, list_size);
      UNE_FOR_RESULT_LIST_ITEM(i, list_size) {
        if (list[i].type == UNE_RT_STR)
          putwc(L'"', stdout);
        une_result_represent(list[i]);
        if (list[i].type == UNE_RT_STR)
          putwc(L'"', stdout);
        if (i < list_size)
          wprintf(L", ");
      }
      putwc(L']', stdout);
      break;
    
    #ifdef UNE_DEBUG
    case UNE_RT_VOID:
      wprintf(L"VOID");
      break;
    
    case UNE_RT_ERROR:
      wprintf(L"ERROR");
      break;
    
    case UNE_RT_CONTINUE:
      wprintf(L"CONTINUE");
      break;
    
    case UNE_RT_BREAK:
      wprintf(L"BREAK");
      break;
    
    case UNE_RT_SIZE:
      wprintf(L"SIZE");
      break;
    #endif
    
  }
}

/*
Return 1 if the une_result is considered 'true', otherwise 0.
*/
une_int une_result_is_true(une_result result)
{
  switch (result.type) {
    case UNE_RT_INT:
      return result.value._int == 0 ? 0 : 1;
    case UNE_RT_FLT:
      return result.value._flt == 0.0 ? 0 : 1;
    case UNE_RT_STR:
      return wcslen(result.value._wcs) == 0 ? 0 : 1;
    case UNE_RT_LIST:
      return ((une_result*)result.value._vp)[0].value._int == 0 ? 0 : 1;
  }
  
  UNE_VERIFY_NOT_REACHED;
}

/*
Return 1 if the une_results are considered equal, otherwise 0.
*/
une_int une_results_are_equal(une_result left, une_result right)
{
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT)
    return left.value._int == right.value._int;
  else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
    return (une_flt)left.value._int == right.value._flt;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
    return left.value._flt == (une_flt)right.value._int;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
    return left.value._flt == right.value._flt;
  
  /* STR and STR. */
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR)
    return !wcscmp(left.value._wcs, right.value._wcs);
  
  /* LIST and LIST. */
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(left, left_list, left_size);
    UNE_UNPACK_RESULT_LIST(right, right_list, right_size);
    if (left_size != right_size)
      return 0;
    UNE_FOR_NODE_LIST_ITEM(i, left_size)
      if (!une_results_are_equal(left_list[i], right_list[i]))
        return 0;
    return 1;
  }
  
  /* Different types; not equal. */
  return 0;
}

/*
Concatenate two une_result lists.
*/
une_result une_result_lists_add(une_result left, une_result right)
{
  /* Unpack result lists. */
  UNE_UNPACK_RESULT_LIST(left, left_list, left_size);
  UNE_UNPACK_RESULT_LIST(right, right_list, right_size);
  
  /* Create new list. */
  une_result *new = une_result_list_create(left_size+right_size);

  /* Populate new list. */
  UNE_FOR_NODE_LIST_ITEM(i, left_size)
    new[i] = une_result_copy(left_list[i]);
  UNE_FOR_NODE_LIST_ITEM(i, right_size)
    new[left_size+i] = une_result_copy(right_list[i]);

  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)new
  };
}

/*
Concatenate two une_result strings.
*/
une_result une_result_strs_add(une_result left, une_result right)
{
  /* Get size of strings. */
  size_t left_size = wcslen(left.value._wcs);
  size_t right_size = wcslen(right.value._wcs);

  /* Create new string. */
  wchar_t *new = malloc((left_size+right_size+1)*sizeof(*new));

  /* Populate new string. */
  wmemcpy(new, left.value._wcs, left_size);
  wmemcpy(new+left_size, right.value._wcs, right_size);
  new[left_size+right_size] = L'\0';

  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = new
  };
}

/*
Repeat a une_result list.
*/
une_result une_result_list_mul(une_result list, une_int count)
{
  /* Unpack source list. */
  UNE_UNPACK_RESULT_LIST(list, list_p, list_size);

  /* Create new list. */
  une_result *new = une_result_list_create(count*list_size+1);

  /* Populate new list. */
  for (size_t i=0; i<count; i++)
    UNE_FOR_RESULT_LIST_ITEM(j, list_size)
      new[i*list_size+j] = une_result_copy(list_p[j]);

  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)new
  };
}

/*
Repeat a une_result string.
*/
une_result une_result_str_mul(une_result str, une_int count)
{
  /* Get size of source string. */
  size_t str_size = wcslen(str.value._wcs);

  /* Create new string. */
  wchar_t *new = malloc((count*str_size+1)*sizeof(*new));

  /* Populate new string. */
  for (size_t i=0; i<count; i++)
    wmemcpy(new+i*str_size, str.value._wcs, str_size);
  new[count*str_size] = L'\0';

  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = new
  };
}
