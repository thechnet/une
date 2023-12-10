/*
function.h - Une
Modified 2023-12-10
*/

#ifndef UNE_TYPES_FUNCTION_H
#define UNE_TYPES_FUNCTION_H

/* Header-specific includes. */
#include "../common.h"
#include "../struct/result.h"
#include "../struct/node.h"
#include "../struct/error.h"
#include "../struct/interpreter_state.h"
#include "../struct/engine.h"

void une_type_function_represent(FILE *file, une_result result);

une_int une_type_function_is_true(une_result result);
une_int une_type_function_is_equal(une_result subject, une_result comparison);

une_result une_type_function_call(une_node *call, une_result function, une_result args, wchar_t *label);

#endif /* UNE_TYPES_FUNCTION_H */
