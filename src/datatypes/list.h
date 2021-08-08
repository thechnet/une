/*
list.h - Une
Modified 2021-08-08
*/

#ifndef UNE_LIST_H
#define UNE_LIST_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../types/result.h"

void une_datatype_list_represent(FILE *file, une_result result);

une_int une_datatype_list_is_true(une_result result);
une_int une_datatype_list_is_equal(une_result subject, une_result comparison);
une_int une_datatype_list_is_greater(une_result subject, une_result comparison);
une_int une_datatype_list_is_greater_or_equal(une_result subject, une_result comparison);
une_int une_datatype_list_is_less(une_result subject, une_result comparison);
une_int une_datatype_list_is_less_or_equal(une_result subject, une_result comparison);

une_result une_datatype_list_add(une_result left, une_result right);
une_result une_datatype_list_mul(une_result left, une_result right);

size_t une_datatype_list_get_len(une_result result);

bool une_datatype_list_is_valid_index_type(une_result_type type);
bool une_datatype_list_is_valid_index(une_result target, une_result index);
une_result une_datatype_list_get_index(une_result target, une_result index);
void une_datatype_list_set_index(une_result *target, une_result index, une_result value);

une_result une_datatype_list_copy(une_result result);
void une_datatype_list_free_members(une_result result);

#endif /* UNE_LIST_H */
