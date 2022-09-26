/*
function.c - Une
Modified 2022-09-26
*/

/* Header-specific includes. */
#include "function.h"

/* Implementation-specific includes. */
#include "../types/symbols.h"
#include "../interpreter.h"

/*
Print a text representation to file.
*/
void une_datatype_function_represent(FILE *file, une_result result)
{
  assert(result.type == UNE_RT_FUNCTION);
  fwprintf(file, L"FUNCTION");
}

/*
Check for truth.
*/
une_int une_datatype_function_is_true(une_result result)
{
  assert(result.type == UNE_RT_FUNCTION);
  return 1;
}

/*
Call result.
*/
une_result une_datatype_function_call(une_error *error, une_interpreter_state *is, une_node *call, une_result function, une_result args)
{
  /* Get function. */
  size_t fn = (size_t)function.value._int;
  assert(fn <= is->functions_size);
  
  /* Ensure number of arguments matches number of required parameters. */
  UNE_UNPACK_RESULT_LIST(args, args_p, args_count);
  if ((is->functions)[fn].params_count != args_count) {
    *error = UNE_ERROR_SET(UNE_ET_CALLABLE_ARG_COUNT, call->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Create function context. */
  une_context *parent = is->context;
  is->context = une_context_create((ptrdiff_t)fn);
  is->context->parent = parent;

  /* Define parameters. */
  for (size_t i=0; i<(is->functions)[fn].params_count; i++) {
    une_variable *var = une_variable_create(is->context, ((is->functions)[fn].params)[i]);
    var->content = une_result_copy(args_p[i+1]);
  }

  /* Interpret body. */
  une_result result = une_interpret(error, is, (is->functions)[fn].body);
  is->should_return = false;

  /* Return to parent context. */
  if (result.type != UNE_RT_ERROR) {
    une_context_free_children(parent, is->context);
    is->context = parent;
  }
  return result;
}
