/*
str.c - Une
Modified 2023-11-17
*/

/* Header-specific includes. */
#include "str.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "str.h"
#include "list.h"

/*
*** Helpers.
*/

static une_reference result_as_strview(une_result subject)
{
	une_result *container;
	if (subject.type == UNE_RT_REFERENCE) {
		if (subject.reference.type == UNE_FT_STRVIEW)
			return subject.reference;
		assert(subject.reference.type == UNE_FT_SINGLE);
		container = (une_result*)subject.reference.root;
	} else {
		assert(subject.type == UNE_RT_STR);
		container = &subject;
	}
	assert(container->type == UNE_RT_STR);
	wchar_t *root = (wchar_t*)container->value._vp;
	size_t width = wcslen(root);
	return (une_reference){
		.type = UNE_FT_STRVIEW,
		.root = root,
		.width = width
	};
}

/*
*** Interface.
*/

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
	
	/* Determine number of repeats. */
	size_t repeat = right.value._int < 0 ? 0 : (size_t)right.value._int;
	
	/* Get size of source string. */
	size_t str_size = wcslen(left.value._wcs);

	/* Create new string. */
	wchar_t *new = malloc((repeat*str_size+1)*sizeof(*new));
	verify(new);

	/* Populate new string. */
	for (size_t i=0; i<repeat; i++)
		wmemcpy(new+i*str_size, left.value._wcs, str_size);
	new[repeat*str_size] = L'\0';

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
Check if an index is valid.
*/
bool une_datatype_str_is_valid_index(une_result subject, une_result index)
{
	if (index.type != UNE_RT_INT)
		return false;
	une_reference strview = result_as_strview(subject);
	une_range range = une_range_from_relative_index(index, strview.width);
	return range.valid;
}

/*
Refer to an element.
*/
une_result une_datatype_str_refer_to_index(une_result subject, une_result index)
{
	une_reference strview = result_as_strview(subject);
	une_range range = une_range_from_relative_index(index, strview.width);
	return (une_result){
		.type = UNE_RT_REFERENCE,
		.reference = (une_reference){
			.type = UNE_FT_STRVIEW,
			.root = (wchar_t*)strview.root + range.first,
			.width = 1
		}
	};
}

/*
Check if a range is valid.
*/
bool une_datatype_str_is_valid_range(une_result subject, une_result begin, une_result end)
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
une_result une_datatype_str_refer_to_range(une_result subject, une_result begin, une_result end)
{
	une_reference strview = result_as_strview(subject);
	une_range range = une_range_from_relative_indices(begin, end, strview.width);
	return (une_result){
		.type = UNE_RT_REFERENCE,
		.reference = (une_reference){
			.type = UNE_FT_STRVIEW,
			.root = (wchar_t*)strview.root + range.first,
			.width = range.length
		}
	};
}

/*
Check if a value can be assigned to a reference.
*/
bool une_datatype_str_can_assign(une_reference subject, une_result value)
{
	if (subject.type == UNE_FT_SINGLE) {
		assert(UNE_RESULT_TYPE_IS_DATA_TYPE(value.type));
		return true;
	}
	assert(subject.type == UNE_FT_STRVIEW);
	assert(value.type == UNE_RT_STR);
	return wcslen(value.value._wcs) == subject.width;
}

/*
Assign a value to a reference.
*/
void une_datatype_str_assign(une_reference subject, une_result value)
{
	if (subject.type == UNE_FT_SINGLE) {
		une_result *root = (une_result*)subject.root;
		une_result_free(*root); /* Free the old value. */
		*root = une_result_copy(value);
		return;
	}
	assert(subject.type == UNE_FT_STRVIEW);
	assert(value.type == UNE_RT_STR);
	wchar_t *destination = (wchar_t*)subject.root;
	wchar_t *source = value.value._wcs;
	assert(wcslen(source) == subject.width);
	for (size_t i=0; i<subject.width; i++)
		destination[i] = source[i];
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
