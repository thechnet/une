/*
interpreter_state.c - Une
Modified 2021-06-13
*/

/* Header-specific includes. */
#include "interpreter_state.h"

/*
Initialize a une_interpreter_state struct.
*/
une_interpreter_state une_interpreter_state_create(une_context *context)
{
  return (une_interpreter_state){
    .context = context,
    .should_return = false
  };
}
