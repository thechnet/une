/*
function.c - Une
Modified 2023-05-13
*/

/* Header-specific includes. */
#include "function.h"

/* Implementation-specific includes. */
#include "../types/association.h"
#include "../interpreter.h"
#include "../tools.h"

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
Create a duplicate.
*/
une_result une_datatype_function_copy(une_result result)
{
  assert(result.type == UNE_RT_FUNCTION);
  
  assert(result.value._vp);
  une_function *source = (une_function*)result.value._vp;
  
  une_function *copy = malloc(sizeof(*copy));
  verify(copy);
  
  assert(source->definition_file);
  copy->definition_file = strdup((char*)source->definition_file);
  verify(copy->definition_file);
  copy->definition_point = source->definition_point;
  
  copy->params_count = source->params_count;
  if (source->params_count > 0) {
    assert(source->params);
    copy->params = malloc(copy->params_count*sizeof(*copy->params));
    verify(copy->params);
    for (size_t i=0; i<source->params_count; i++) {
      assert(source->params[i]);
      copy->params[i] = wcsdup(source->params[i]);
      verify(copy->params[i]);
    }
  } else {
    copy->params = NULL;
  }
  
  assert(source->body);
  copy->body = une_node_copy(source->body);
  
  return (une_result){
    .type = UNE_RT_FUNCTION,
    .value._vp = (void*)copy
  };
}

/*
Free all members.
*/
void une_datatype_function_free_members(une_result result)
{
  assert(result.type == UNE_RT_FUNCTION);
  
  assert(result.value._vp);
  une_function *function = (une_function*)result.value._vp;
  
  assert(function->definition_file);
  free(function->definition_file);
  
  if (function->params_count > 0) {
    assert(function->params);
    for (size_t i=0; i<function->params_count; i++) {
      assert(function->params[i]);
      free(function->params[i]);
    }
    free(function->params);
  }
  
  assert(function->body);
  une_node_free(function->body, true);
  
  free(function);
}

/*
Call result.
*/
une_result une_datatype_function_call(une_error *error, une_node *call, une_result function, une_result args)
{
  /* Get function. */
  assert(function.type == UNE_RT_FUNCTION);
  assert(function.value._vp);
  une_function *callee = (une_function*)function.value._vp;
  
  /* Ensure number of arguments matches number of required parameters. */
  UNE_UNPACK_RESULT_LIST(args, args_p, args_count);
  if (callee->params_count != args_count) {
    *error = UNE_ERROR_SET(UNE_ET_CALLABLE_ARG_COUNT, call->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Create function context. */
  une_context *parent = une_is->context;
  char *entry_file = strdup(callee->definition_file);
  verify(entry_file);
  une_is->context = une_context_create(entry_file, callee->definition_point);
  une_is->context->parent = parent;

  /* Define parameters. */
  for (size_t i=0; i<callee->params_count; i++) {
    une_association *var = une_variable_create(une_is->context, (callee->params)[i]);
    var->content = une_result_copy(args_p[i+1]);
  }

  /* Interpret body. */
  une_result result = une_interpret(error, callee->body);
  une_is->should_return = false;

  /* Return to parent context. */
  if (result.type != UNE_RT_ERROR) {
    une_context_free_children(parent, une_is->context);
    une_is->context = parent;
  }
  return result;
}
