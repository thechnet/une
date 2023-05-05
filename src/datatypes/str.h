/*
str.h - Une
Modified 2023-05-04
*/

#ifndef UNE_DATATYPES_STR_H
#define UNE_DATATYPES_STR_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../types/result.h"

une_result une_datatype_str_as_int(une_result result);
une_result une_datatype_str_as_flt(une_result result);
une_result une_datatype_str_as_str(une_result result);
void une_datatype_str_represent(FILE *file, une_result result);

une_int une_datatype_str_is_true(une_result result);
une_int une_datatype_str_is_equal(une_result subject, une_result comparison);
une_int une_datatype_str_is_greater(une_result subject, une_result comparison);
une_int une_datatype_str_is_greater_or_equal(une_result subject, une_result comparison);
une_int une_datatype_str_is_less(une_result subject, une_result comparison);
une_int une_datatype_str_is_less_or_equal(une_result subject, une_result comparison);

une_result une_datatype_str_add(une_result left, une_result right);
une_result une_datatype_str_mul(une_result left, une_result right);

size_t une_datatype_str_get_len(une_result result);

bool une_datatype_str_is_valid_index(une_result subject, une_result index);
une_result une_datatype_str_refer_to_index(une_result subject, une_result index);
bool une_datatype_str_is_valid_range(une_result subject, une_result begin, une_result end);
une_result une_datatype_str_refer_to_range(une_result subject, une_result begin, une_result end);
bool une_datatype_str_can_assign(une_reference subject, une_result value);
void une_datatype_str_assign(une_reference subject, une_result value);

une_result une_datatype_str_copy(une_result result);
void une_datatype_str_free_members(une_result result);

#endif /* UNE_DATATYPES_STR_H */
