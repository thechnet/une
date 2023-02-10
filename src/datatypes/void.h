/*
void.h - Une
Modified 2023-02-10
*/

#ifndef UNE_DATATYPES_VOID_H
#define UNE_DATATYPES_VOID_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../types/result.h"

void une_datatype_void_represent(FILE *file, une_result result);

une_int une_datatype_void_is_true(une_result result);
une_int une_datatype_void_is_equal(une_result subject, une_result comparison);

#endif /* UNE_DATATYPES_VOID_H */
