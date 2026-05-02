/*
function.h - Une
*/

#ifndef UNE_TYPES_FUNCTION_H
#define UNE_TYPES_FUNCTION_H

/* Header-specific includes. */
#include "../common.h"
#include "../struct/engine.h"
#include "../struct/error.h"
#include "../struct/interpreter_state.h"
#include "../struct/node.h"
#include "../struct/result.h"

void une_type_function_represent(FILE *file, une_result result);

une_int une_type_function_is_true(une_result result);
une_int une_type_function_is_equal(une_result subject, une_result comparison);

une_result
une_type_function_call(une_node *call, une_result function, une_result args, wchar_t *label);

#endif /* UNE_TYPES_FUNCTION_H */
