/*
function.h - Une
Modified 2023-11-19
*/

#ifndef UNE_DATATYPES_FUNCTION_H
#define UNE_DATATYPES_FUNCTION_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../types/result.h"
#include "../types/node.h"
#include "../types/error.h"
#include "../types/interpreter_state.h"
#include "../types/engine.h"

void une_datatype_function_represent(FILE *file, une_result result);

une_int une_datatype_function_is_true(une_result result);

une_result une_datatype_function_call(une_node *call, une_result function, une_result args, wchar_t *label);

#endif /* UNE_DATATYPES_FUNCTION_H */
