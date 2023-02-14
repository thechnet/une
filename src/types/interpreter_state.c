/*
interpreter_state.c - Une
Modified 2023-02-14
*/

/* Header-specific includes. */
#include "interpreter_state.h"
#include "../tools.h"

/*
Initialize a une_interpreter_state struct.
*/
une_interpreter_state une_interpreter_state_create(void)
{
  une_function *functions = malloc(UNE_SIZE_FUNCTION_BUF*sizeof(*functions));
  verify(functions);
  return (une_interpreter_state){
    .context = une_context_create(-1),
    .should_return = false,
    .should_exit = false,
    .functions = functions,
    .functions_size = UNE_SIZE_FUNCTION_BUF,
    .functions_count = 0,
    .this_contestant = une_result_create(UNE_RT_VOID),
    .this = une_result_create(UNE_RT_VOID)
  };
}

/*
Free all members of a une_interpreter_state struct.
*/
void une_interpreter_state_free(une_interpreter_state *is)
{
  /* Context. */
  une_context_free_children(NULL, is->context);
  
  /* Functions. */
  assert(is->functions != NULL);
  for (size_t i=0; i<is->functions_count; i++)
    une_function_free(is->functions+i);
  free(is->functions);
  
  /* 'this' contestant. */
  une_result_free(is->this_contestant);
}

/*
Initializes a new une_function in a une_interpreter_state's function buffer.
*/
size_t une_function_create(une_interpreter_state *is, char *definition_file, une_position definition_point)
{
  /* Ensure sufficient space in une_function buffer. */
  if (is->functions_count >= is->functions_size) {
    is->functions_size *= 2;
    is->functions = realloc(is->functions, is->functions_size*sizeof(*is->functions));
    verify(is->functions);
  }
  
  /* Initialize une_function. */
  une_function fn = (une_function){
    .definition_file = strdup(definition_file),
    .definition_point = definition_point,
    .params_count = 0,
    .params = NULL,
    .body = NULL
  };
  verify(fn.definition_file);
  (is->functions)[is->functions_count] = fn;
  
  return is->functions_count++;
}
