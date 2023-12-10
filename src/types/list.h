/*
list.h - Une
Modified 2023-12-10
*/

#ifndef UNE_TYPES_LIST_H
#define UNE_TYPES_LIST_H

/* Header-specific includes. */
#include "../common.h"
#include "../struct/result.h"

void une_type_list_represent(FILE *file, une_result result);

une_int une_type_list_is_true(une_result result);
une_int une_type_list_is_equal(une_result subject, une_result comparison);
une_int une_type_list_is_greater(une_result subject, une_result comparison);
une_int une_type_list_is_greater_or_equal(une_result subject, une_result comparison);
une_int une_type_list_is_less(une_result subject, une_result comparison);
une_int une_type_list_is_less_or_equal(une_result subject, une_result comparison);

une_result une_type_list_add(une_result left, une_result right);
une_result une_type_list_mul(une_result left, une_result right);

size_t une_type_list_get_len(une_result result);

bool une_type_list_is_valid_index(une_result subject, une_result index);
une_result une_type_list_refer_to_index(une_result subject, une_result index);
bool une_type_list_is_valid_range(une_result subject, une_result begin, une_result end);
une_result une_type_list_refer_to_range(une_result subject, une_result begin, une_result end);
bool une_type_list_can_assign(une_reference subject, une_result value);
void une_type_list_assign(une_reference subject, une_result value);

une_result une_type_list_copy(une_result result);
void une_type_list_free_members(une_result result);

#endif /* UNE_TYPES_LIST_H */
