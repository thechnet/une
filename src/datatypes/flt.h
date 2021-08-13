/*
flt.h - Une
Modified 2021-08-11
*/

#ifndef UNE_DATATYPES_FLT_H
#define UNE_DATATYPES_FLT_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../types/result.h"

une_result une_datatype_flt_as_int(une_result result);
une_result une_datatype_flt_as_flt(une_result result);
une_result une_datatype_flt_as_str(une_result result);
void une_datatype_flt_represent(FILE *file, une_result result);

une_int une_datatype_flt_is_true(une_result result);
une_int une_datatype_flt_is_equal(une_result subject, une_result comparison);
une_int une_datatype_flt_is_greater(une_result subject, une_result comparison);
une_int une_datatype_flt_is_greater_or_equal(une_result subject, une_result comparison);
une_int une_datatype_flt_is_less(une_result subject, une_result comparison);
une_int une_datatype_flt_is_less_or_equal(une_result subject, une_result comparison);

une_result une_datatype_flt_add(une_result left, une_result right);
une_result une_datatype_flt_sub(une_result left, une_result right);
une_result une_datatype_flt_mul(une_result left, une_result right);
une_result une_datatype_flt_div(une_result left, une_result right);
une_result une_datatype_flt_fdiv(une_result left, une_result right);
une_result une_datatype_flt_mod(une_result left, une_result right);
une_result une_datatype_flt_pow(une_result left, une_result right);
une_result une_datatype_flt_negate(une_result result);

#endif /* UNE_DATATYPES_FLT_H */
