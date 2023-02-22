/*
interpreter_state.c - Une
Modified 2023-02-22
*/

/* Header-specific includes. */
#include "interpreter_state.h"
#include "../tools.h"

/*
Initialize a une_interpreter_state struct.
*/
une_interpreter_state une_interpreter_state_create(void)
{
  return (une_interpreter_state){
    .context = une_context_create(NULL, (une_position){0}),
    .should_return = false,
    .should_exit = false,
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
  
  /* 'this' contestant. */
  une_result_free(is->this_contestant);
}
