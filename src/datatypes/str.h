/*
str.h - Une
Modified 2023-02-10
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

bool une_datatype_str_is_valid_index_type(une_result_type type);
bool une_datatype_str_is_valid_index(une_result target, une_result index);
bool une_datatype_str_is_valid_element(une_result element);
une_result une_datatype_str_get_index(une_result target, une_result index);
une_result une_datatype_str_seek_index(une_result *target, une_result index);

une_result une_datatype_str_copy(une_result result);
void une_datatype_str_free_members(une_result result);

#endif /* UNE_DATATYPES_STR_H */
