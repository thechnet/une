/*
function.h - Une
Modified 2023-02-22
*/

#ifndef UNE_DATATYPES_FUNCTION_H
#define UNE_DATATYPES_FUNCTION_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../types/result.h"
#include "../types/node.h"
#include "../types/error.h"
#include "../types/interpreter_state.h"

void une_datatype_function_represent(FILE *file, une_result result);

une_int une_datatype_function_is_true(une_result result);

une_result une_datatype_function_copy(une_result result);
void une_datatype_function_free_members(une_result result);

une_result une_datatype_function_call(une_error *error, une_interpreter_state *is, une_node *call, une_result function, une_result args);

/*
Function.
*/

typedef struct une_function_ {
  char *definition_file;
  une_position definition_point;
  size_t params_count;
  wchar_t **params;
  une_node *body;
} une_function;

#endif /* UNE_DATATYPES_FUNCTION_H */
