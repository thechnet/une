/*
list.c - Une
Modified 2023-06-19
*/

/* Header-specific includes. */
#include "list.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
*** Helpers.
*/

static une_reference result_as_listview(une_result subject)
{
  une_result *container;
  if (subject.type == UNE_RT_REFERENCE) {
    if (subject.reference.type == UNE_FT_LISTVIEW)
      return subject.reference;
    assert(subject.reference.type == UNE_FT_SINGLE);
    container = (une_result*)subject.reference.root;
  } else {
    assert(subject.type == UNE_RT_LIST);
    container = &subject;
  }
  assert(container->type == UNE_RT_LIST);
  une_result *root = (une_result*)container->value._vp;
  assert(root[0].type == UNE_RT_SIZE);
  size_t width = (size_t)root++[0].value._int;
  return (une_reference){
    .type = UNE_FT_LISTVIEW,
    .root = root,
    .width = width
  };
}

/*
*** Interface.
*/

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
    if (!une_result_equ_result(subject_list[i], comparison_list[i]))
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
  
  /* Determine number of repeats. */
  size_t repeat = right.value._int < 0 ? 0 : (size_t)right.value._int;
  
  /* Unpack source list. */
  UNE_UNPACK_RESULT_LIST(left, left_p, left_size);

  /* Create new list. */
  une_result *new = une_result_list_create(repeat*left_size);

  /* Populate new list. */
  for (size_t i=0; i<repeat; i++)
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
Check if an index is valid.
*/
bool une_datatype_list_is_valid_index(une_result subject, une_result index)
{
  if (index.type != UNE_RT_INT)
    return false;
  une_reference listview = result_as_listview(subject);
  une_range range = une_range_from_relative_index(index, listview.width);
  return range.valid;
}

/*
Refer to an element.
*/
une_result une_datatype_list_refer_to_index(une_result subject, une_result index)
{
  une_reference listview = result_as_listview(subject);
  une_range range = une_range_from_relative_index(index, listview.width);
  return (une_result){
    .type = UNE_RT_REFERENCE,
    .reference = (une_reference){
      .type = UNE_FT_SINGLE,
      .root = (une_result*)listview.root + range.first
    }
  };
}

/*
Check if a range is valid.
*/
bool une_datatype_list_is_valid_range(une_result subject, une_result begin, une_result end)
{
  if (begin.type != UNE_RT_INT)
    return false;
  if (end.type != UNE_RT_INT && end.type != UNE_RT_VOID)
    return false;
  return true;
}

/*
Refer to a range of elements.
*/
une_result une_datatype_list_refer_to_range(une_result subject, une_result begin, une_result end)
{
  une_reference listview = result_as_listview(subject);
  une_range range = une_range_from_relative_indices(begin, end, listview.width);
  return (une_result){
    .type = UNE_RT_REFERENCE,
    .reference = (une_reference){
      .type = UNE_FT_LISTVIEW,
      .root = (une_result*)listview.root + range.first,
      .width = range.length
    }
  };
}

/*
Check if a value can be assigned to a reference.
*/
bool une_datatype_list_can_assign(une_reference subject, une_result value)
{
  if (subject.type == UNE_FT_SINGLE) {
    assert(UNE_RESULT_TYPE_IS_DATA_TYPE(value.type));
    return true;
  }
  assert(subject.type == UNE_FT_LISTVIEW);
  assert(value.type == UNE_RT_LIST);
  UNE_UNPACK_RESULT_LIST(value, value_list, value_size);
  return value_size == subject.width;
}

/*
Assign a value to a reference.
*/
void une_datatype_list_assign(une_reference subject, une_result value)
{
  if (subject.type == UNE_FT_SINGLE) {
    une_result *root = (une_result*)subject.root;
    une_result_free(*root); /* Free the old value. */
    *root = une_result_copy(value);
    return;
  }
  assert(subject.type == UNE_FT_LISTVIEW);
  assert(value.type == UNE_RT_LIST);
  une_result *destination = (une_result*)subject.root;
  UNE_UNPACK_RESULT_LIST(value, source, source_size);
  assert(source_size == subject.width);
  UNE_FOR_RESULT_LIST_ITEM(i, source_size) {
    une_result_free(destination[i-1]); /* Free the old value. */
    destination[i-1] = une_result_copy(source[i]);
  }
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
