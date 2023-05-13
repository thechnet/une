/*
builtin.h - Une
Modified 2023-05-13
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

une_result une_datatype_builtin_call(une_error *error, une_node *call, une_result function, une_result args);

#endif /* UNE_DATATYPES_BUILTIN_H */
