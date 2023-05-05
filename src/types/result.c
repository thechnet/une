/*
result.c - Une
Modified 2023-05-05
*/

/* Header-specific includes. */
#include "result.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "node.h"
#include "../datatypes/datatypes.h"

/*
Result name table.
*/
const wchar_t *une_result_table[] = {
  L"ERROR",
  L"VOID",
  L"INT",
  L"FLT",
  L"STR",
  L"LIST",
  L"OBJECT",
  L"FUNCTION",
  L"BUILTIN",
  L"CONTINUE",
  L"BREAK",
  L"SIZE",
  L"REFERENCE",
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
une_result une_result_copy(une_result original)
{
  assert(UNE_RESULT_TYPE_IS_VALID(original.type));
  if (!UNE_RESULT_TYPE_IS_DATA_TYPE(original.type))
    return original;
  une_datatype dt_original = UNE_DATATYPE_FOR_RESULT(original);
  if (dt_original.copy == NULL)
    return original;
  return dt_original.copy(original);
}

/*
Free all members of a une_result.
*/
void une_result_free(une_result result)
{
  assert(UNE_RESULT_TYPE_IS_VALID(result.type));
  if (UNE_RESULT_TYPE_IS_DATA_TYPE(result.type))
    if (UNE_DATATYPE_FOR_RESULT(result).free_members != NULL)
      UNE_DATATYPE_FOR_RESULT(result).free_members(result);
}

/*
Return a list of uninitialized une_results.
*/
une_result *une_result_list_create(size_t items)
{
  une_result *list = malloc((items+1)*sizeof(*list));
  verify(list);
  list[0] = (une_result){
    .type = UNE_RT_SIZE,
    .value._int = (une_int)items
  };
  return list;
}

/*
Return a text representation of a une_result_type.
*/
#ifdef UNE_DEBUG
const wchar_t *une_result_type_to_wcs(une_result_type type)
{
  assert(UNE_RESULT_TYPE_IS_VALID(type));
  return une_result_table[type-1];
}
#endif /* UNE_DEBUG */

/*
Print a text representation of a une_result.
*/
void une_result_represent(FILE *file, une_result result)
{
  assert(UNE_RESULT_TYPE_IS_DATA_TYPE(result.type));
  assert(UNE_DATATYPE_FOR_RESULT(result).represent != NULL);
  UNE_DATATYPE_FOR_RESULT(result).represent(file, result);
}

/*
Return 1 if the une_result is considered 'true', otherwise 0.
*/
une_int une_result_is_true(une_result result)
{
  assert(UNE_RESULT_TYPE_IS_DATA_TYPE(result.type));
  assert(UNE_DATATYPE_FOR_RESULT(result).is_true != NULL);
  return UNE_DATATYPE_FOR_RESULT(result).is_true(result);
}

/*
Return 1 if the une_results are considered equal, otherwise 0.
*/
une_int une_results_are_equal(une_result left, une_result right)
{
  assert(UNE_RESULT_TYPE_IS_DATA_TYPE(left.type));
  assert(UNE_RESULT_TYPE_IS_DATA_TYPE(right.type));
  if (UNE_DATATYPE_FOR_RESULT(left).is_equal == NULL)
    return 0;
  return UNE_DATATYPE_FOR_RESULT(left).is_equal(left, right);
}

/*
Dereference reference results.
*/
une_result une_result_dereference(une_result result)
{
  if (result.type != UNE_RT_REFERENCE)
    return result;
  assert(UNE_REFERENCE_TYPE_IS_VALID(result.reference.type));
  switch (result.reference.type) {
    case UNE_FT_STRVIEW: {
      wchar_t *strview = (wchar_t*)result.reference.root;
      wchar_t *string = malloc((result.reference.width+1)*sizeof(*string));
      verify(string);
      wcsncpy(string, strview, result.reference.width);
      string[result.reference.width] = L'\0';
      return (une_result){
        .type = UNE_RT_STR,
        .value._wcs = string
      };
    }
    case UNE_FT_LISTVIEW: {
      une_result *listview = (une_result*)result.reference.root;
      une_result *list = une_result_list_create(result.reference.width);
      UNE_FOR_RESULT_LIST_ITEM(i, result.reference.width)
        list[i] = une_result_copy(listview[i-1]) /* Listviews don't know their size. */;
      return (une_result){
        .type = UNE_RT_LIST,
        .value._vp = (void*)list
      };
    }
  }
  assert(result.reference.type == UNE_FT_SINGLE);
  une_result *referenced = (une_result*)result.reference.root;
  assert(referenced);
  assert(UNE_RESULT_TYPE_IS_DATA_TYPE(referenced->type));
  return une_result_copy(*referenced);
}
