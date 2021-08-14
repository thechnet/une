/*
str.c - Une
Modified 2021-08-14
*/

/* Header-specific includes. */
#include "str.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "str.h"
#include "list.h"

/*
Convert to INT.
*/
une_result une_datatype_str_as_int(une_result result)
{
  assert(result.type == UNE_RT_STR);
  une_int int_;
  if (!une_wcs_to_une_int(result.value._wcs, &int_))
    return une_result_create(UNE_RT_ERROR);
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = int_
  };
}

/*
Convert to FLT.
*/
une_result une_datatype_str_as_flt(une_result result)
{
  assert(result.type == UNE_RT_STR);
  une_flt flt_;
  if (!une_wcs_to_une_flt(result.value._wcs, &flt_))
    return une_result_create(UNE_RT_ERROR);
  return (une_result){
    .type = UNE_RT_FLT,
    .value._flt = flt_
  };
}

/*
Convert to STR.
*/
une_result une_datatype_str_as_str(une_result result)
{
  assert(result.type == UNE_RT_STR);
  return une_datatype_str_copy(result);
}

/*
Print a text representation to file.
*/
void une_datatype_str_represent(FILE *file, une_result result)
{
  assert(result.type == UNE_RT_STR);
  fwprintf(file, L"%ls", result.value._wcs);
}

/*
Check for truth.
*/
une_int une_datatype_str_is_true(une_result result)
{
  assert(result.type == UNE_RT_STR);
  return wcslen(result.value._wcs) == 0 ? 0 : 1;
}

/*
Check if subject is equal to comparison.
*/
une_int une_datatype_str_is_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_STR);
  if (comparison.type != UNE_RT_STR)
    return 0;
  return wcscmp(subject.value._wcs, comparison.value._wcs) == 0;
}

/*
Check if subject is greater than comparison.
*/
une_int une_datatype_str_is_greater(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_STR);
  if (comparison.type == UNE_RT_STR)
    return wcslen(subject.value._wcs) > wcslen(comparison.value._wcs);
  return -1;
}

/*
Check if subject is greater than or equal to comparison.
*/
une_int une_datatype_str_is_greater_or_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_STR);
  if (comparison.type == UNE_RT_STR)
    return wcslen(subject.value._wcs) >= wcslen(comparison.value._wcs);
  return -1;
}

/*
Check is subject is less than comparison.
*/
une_int une_datatype_str_is_less(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_STR);
  if (comparison.type == UNE_RT_STR)
    return wcslen(subject.value._wcs) < wcslen(comparison.value._wcs);
  return -1;
}

/*
Check if subject is less than or equal to comparison.
*/
une_int une_datatype_str_is_less_or_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_STR);
  if (comparison.type == UNE_RT_STR)
    return wcslen(subject.value._wcs) <= wcslen(comparison.value._wcs);
  return -1;
}

/*
Add right to left.
*/
une_result une_datatype_str_add(une_result left, une_result right)
{
  assert(left.type == UNE_RT_STR);
  if (right.type != UNE_RT_STR)
    return une_result_create(UNE_RT_ERROR);
  
  /* Get size of strings. */
  size_t left_size = wcslen(left.value._wcs);
  size_t right_size = wcslen(right.value._wcs);

  /* Create new string. */
  wchar_t *new = malloc((left_size+right_size+1)*sizeof(*new));
  verify(new);

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
Multiply left by right.
*/
une_result une_datatype_str_mul(une_result left, une_result right)
{
  assert(left.type == UNE_RT_STR);
  if (right.type != UNE_RT_INT)
    return une_result_create(UNE_RT_ERROR);
  
  /* Get size of source string. */
  size_t str_size = wcslen(left.value._wcs);

  /* Create new string. */
  wchar_t *new = malloc((right.value._int*str_size+1)*sizeof(*new));
  verify(new);

  /* Populate new string. */
  for (size_t i=0; i<right.value._int; i++)
    wmemcpy(new+i*str_size, left.value._wcs, str_size);
  new[right.value._int*str_size] = L'\0';

  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = new
  };
}

/*
Get the length.
*/
size_t une_datatype_str_get_len(une_result result)
{
  assert(result.type == UNE_RT_STR);
  return wcslen(result.value._wcs);
}

/*
Check if a index type is valid.
*/
bool une_datatype_str_is_valid_index_type(une_result_type type)
{
  return type == UNE_RT_INT;
}

/*
Check if a index is valid.
*/
bool une_datatype_str_is_valid_index(une_result target, une_result index)
{
  assert(index.type == UNE_RT_INT);
  return index.value._int >= 0 && index.value._int < une_datatype_str_get_len(target);
}

/*
Get the value at index.
*/
une_result une_datatype_str_get_index(une_result target, une_result index)
{
  assert(target.type == UNE_RT_STR);
  assert(index.type == UNE_RT_INT);
  wchar_t *substring = malloc(2*sizeof(*substring));
  verify(substring);
  substring[0] = target.value._wcs[index.value._int];
  substring[1] = L'\0';
  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = substring
  };
}

/*
Create a duplicate.
*/
une_result une_datatype_str_copy(une_result result)
{
  assert(result.type == UNE_RT_STR);
  une_result copy = {
    .type = UNE_RT_STR,
    .value._wcs = wcsdup(result.value._wcs)
  };
  verify(copy.value._wcs);
  return copy;
}

/*
Free all members.
*/
void une_datatype_str_free_members(une_result result)
{
  free(result.value._wcs);
}
