/*
builtin.h - Une
Modified 2023-12-01
*/

#ifndef UNE_DATATYPES_BUILTIN_H
#define UNE_DATATYPES_BUILTIN_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../types/result.h"
#include "../types/node.h"
#include "../types/error.h"
#include "../types/interpreter_state.h"

void une_datatype_builtin_represent(FILE *file, une_result result);

une_int une_datatype_builtin_is_true(une_result result);
une_int une_datatype_builtin_is_equal(une_result subject, une_result comparison);

une_result une_datatype_builtin_call(une_node *call, une_result function, une_result args, wchar_t *label);

#endif /* UNE_DATATYPES_BUILTIN_H */
