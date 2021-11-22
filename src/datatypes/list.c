/*
list.c - Une
Modified 2021-11-22
*/

/* Header-specific includes. */
#include "list.h"

/*
Print a text representation to file.
*/
void une_datatype_list_represent(FILE *file, une_result result)
{
  assert(result.type == UNE_RT_LIST);
  fwprintf(file, L"[");
  UNE_UNPACK_RESULT_LIST(result, list, list_size);
  UNE_FOR_RESULT_LIST_ITEM(i, list_size) {
    if (list[i].type == UNE_RT_STR)
      putwc(L'"', file);
    une_result_represent(file, list[i]);
    if (list[i].type == UNE_RT_STR)
      putwc(L'"', file);
    if (i < list_size)
      fwprintf(file, L", ");
  }
  putwc(L']', file);
}

/*
Check for truth.
*/
une_int une_datatype_list_is_true(une_result result)
{
  assert(result.type == UNE_RT_LIST);
  return ((une_result*)result.value._vp)[0].value._int == 0 ? 0 : 1;
}

/*
Check if subject is equal to comparison.
*/
une_int une_datatype_list_is_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_LIST);
  if (comparison.type != UNE_RT_LIST)
    return 0;
  UNE_UNPACK_RESULT_LIST(subject, subject_list, subject_size);
  UNE_UNPACK_RESULT_LIST(comparison, comparison_list, comparison_size);
  if (subject_size != comparison_size)
    return 0;
  UNE_FOR_RESULT_LIST_ITEM(i, subject_size)
    if (!une_results_are_equal(subject_list[i], comparison_list[i]))
      return 0;
  return 1;
}

/*
Check if subject is greater than comparison.
*/
une_int une_datatype_list_is_greater(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_LIST);
  if (comparison.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(subject, subject_list, subject_list_size);
    UNE_UNPACK_RESULT_LIST(comparison, comparison_list, comparison_list_size);
    return subject_list_size > comparison_list_size;
  }
  return -1;
}

/*
Check if subject is greater than or equal to comparison.
*/
une_int une_datatype_list_is_greater_or_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_LIST);
  if (comparison.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(subject, subject_list, subject_list_size);
    UNE_UNPACK_RESULT_LIST(comparison, comparison_list, comparison_list_size);
    return subject_list_size >= comparison_list_size;
  }
  return -1;
}

/*
Check is subject is less than comparison.
*/
une_int une_datatype_list_is_less(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_LIST);
  if (comparison.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(subject, subject_list, subject_list_size);
    UNE_UNPACK_RESULT_LIST(comparison, comparison_list, comparison_list_size);
    return subject_list_size < comparison_list_size;
  }
  return -1;
}

/*
Check if subject is less than or equal to comparison.
*/
une_int une_datatype_list_is_less_or_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_LIST);
  if (comparison.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(subject, subject_list, subject_list_size);
    UNE_UNPACK_RESULT_LIST(comparison, comparison_list, comparison_list_size);
    return subject_list_size <= comparison_list_size;
  }
  return -1;
}

/*
Add right to left.
*/
une_result une_datatype_list_add(une_result left, une_result right)
{
  assert(left.type == UNE_RT_LIST);
  if (right.type != UNE_RT_LIST)
    return une_result_create(UNE_RT_ERROR);
  
  /* Unpack result lists. */
  UNE_UNPACK_RESULT_LIST(left, left_list, left_size);
  UNE_UNPACK_RESULT_LIST(right, right_list, right_size);
  
  /* Create new list. */
  une_result *new = une_result_list_create(left_size+right_size);

  /* Populate new list. */
  UNE_FOR_RESULT_LIST_ITEM(i, left_size)
    new[i] = une_result_copy(left_list[i]);
  UNE_FOR_RESULT_LIST_ITEM(i, right_size)
    new[left_size+i] = une_result_copy(right_list[i]);

  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)new
  };
}

/*
Multiply left by right.
*/
une_result une_datatype_list_mul(une_result left, une_result right)
{
  assert(left.type == UNE_RT_LIST);
  if (right.type != UNE_RT_INT)
    return une_result_create(UNE_RT_ERROR);
  
  /* Unpack source list. */
  UNE_UNPACK_RESULT_LIST(left, left_p, left_size);

  /* Create new list. */
  une_result *new = une_result_list_create(right.value._int*left_size);

  /* Populate new list. */
  for (une_int i=0; i<right.value._int; i++)
    UNE_FOR_RESULT_LIST_ITEM(j, left_size)
      new[i*left_size+j] = une_result_copy(left_p[j]);

  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)new
  };
}

/*
Get the length.
*/
size_t une_datatype_list_get_len(une_result result)
{
  assert(result.type == UNE_RT_LIST);
  UNE_UNPACK_RESULT_LIST(result, result_p, result_size);
  return result_size;
}

/*
Check if an index type is valid.
*/
bool une_datatype_list_is_valid_index_type(une_result_type type)
{
  return type == UNE_RT_INT;
}

/*
Check if an index is valid.
*/
bool une_datatype_list_is_valid_index(une_result target, une_result index)
{
  assert(index.type == UNE_RT_INT);
  return index.value._int >= 0 && (_une_uint)index.value._int < une_datatype_list_get_len(target);
}

/*
Get the value at index.
*/
une_result une_datatype_list_get_index(une_result target, une_result index)
{
  assert(target.type == UNE_RT_LIST);
  assert(index.type == UNE_RT_INT);
  return une_result_copy(((une_result*)target.value._vp)[1+index.value._int]);
}

/*
Set the value at index.
*/
void une_datatype_list_set_index(une_result *target, une_result index, une_result value)
{
  assert(target->type == UNE_RT_LIST);
  assert(index.type == UNE_RT_INT);
  une_result_free(((une_result*)target->value._vp)[1+index.value._int]);
  ((une_result*)target->value._vp)[1+index.value._int] = value;
}

/*
Create a duplicate.
*/
une_result une_datatype_list_copy(une_result original)
{
  assert(original.type == UNE_RT_LIST);
  UNE_UNPACK_RESULT_LIST(original, original_list, size);
  une_result *copy = une_result_list_create(size);
  UNE_FOR_RESULT_LIST_ITEM(i, size)
    copy[i] = une_result_copy(original_list[i]);
  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)copy
  };
}

/*
Free all members.
*/
void une_datatype_list_free_members(une_result result)
{
  assert(result.type == UNE_RT_LIST);
  UNE_UNPACK_RESULT_LIST(result, list, list_size);
  UNE_FOR_RESULT_LIST_INDEX(i, list_size)
    une_result_free(list[i]);
  free(list);
}
