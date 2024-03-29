/*
void.h - Une
Modified 2023-12-10
*/

#ifndef UNE_TYPES_VOID_H
#define UNE_TYPES_VOID_H

/* Header-specific includes. */
#include "../common.h"
#include "../struct/result.h"

void une_type_void_represent(FILE *file, une_result result);

une_int une_type_void_is_true(une_result result);
une_int une_type_void_is_equal(une_result subject, une_result comparison);

#endif /* UNE_TYPES_VOID_H */
