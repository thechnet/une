/*
flt.h - Une
Modified 2023-12-10
*/

#ifndef UNE_TYPES_FLT_H
#define UNE_TYPES_FLT_H

/* Header-specific includes. */
#include "../common.h"
#include "../struct/result.h"

une_result une_type_flt_as_int(une_result result);
une_result une_type_flt_as_flt(une_result result);
une_result une_type_flt_as_str(une_result result);
void une_type_flt_represent(FILE *file, une_result result);

une_int une_type_flt_is_true(une_result result);
une_int une_type_flt_is_equal(une_result subject, une_result comparison);
une_int une_type_flt_is_greater(une_result subject, une_result comparison);
une_int une_type_flt_is_greater_or_equal(une_result subject, une_result comparison);
une_int une_type_flt_is_less(une_result subject, une_result comparison);
une_int une_type_flt_is_less_or_equal(une_result subject, une_result comparison);

une_result une_type_flt_add(une_result left, une_result right);
une_result une_type_flt_sub(une_result left, une_result right);
une_result une_type_flt_mul(une_result left, une_result right);
une_result une_type_flt_div(une_result left, une_result right);
une_result une_type_flt_fdiv(une_result left, une_result right);
une_result une_type_flt_mod(une_result left, une_result right);
une_result une_type_flt_pow(une_result left, une_result right);
une_result une_type_flt_negate(une_result result);

#endif /* UNE_TYPES_FLT_H */
