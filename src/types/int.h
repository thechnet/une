/*
int.h - Une
Modified 2023-12-10
*/

#ifndef UNE_TYPES_INT_H
#define UNE_TYPES_INT_H

/* Header-specific includes. */
#include "../common.h"
#include "../struct/result.h"

une_result une_type_int_as_int(une_result result);
une_result une_type_int_as_flt(une_result result);
une_result une_type_int_as_str(une_result result);
void une_type_int_represent(FILE *file, une_result result);

une_int une_type_int_is_true(une_result result);
une_int une_type_int_is_equal(une_result subject, une_result comparison);
une_int une_type_int_is_greater(une_result subject, une_result comparison);
une_int une_type_int_is_greater_or_equal(une_result subject, une_result comparison);
une_int une_type_int_is_less(une_result subject, une_result comparison);
une_int une_type_int_is_less_or_equal(une_result subject, une_result comparison);

une_result une_type_int_add(une_result left, une_result right);
une_result une_type_int_sub(une_result left, une_result right);
une_result une_type_int_mul(une_result left, une_result right);
une_result une_type_int_div(une_result left, une_result right);
une_result une_type_int_fdiv(une_result left, une_result right);
une_result une_type_int_mod(une_result left, une_result right);
une_result une_type_int_pow(une_result left, une_result right);
une_result une_type_int_negate(une_result result);

#endif /* UNE_TYPES_INT_H */
