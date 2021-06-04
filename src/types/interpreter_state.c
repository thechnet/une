/*
interpreter_state.c - Une
Modified 2021-06-04
*/

#include "interpreter_state.h"

#pragma region une_interpreter_state_create
une_interpreter_state une_interpreter_state_create(une_context *context)
{
  return (une_interpreter_state){
    .context = context,
    .should_return = false
  };
}
#pragma endregion une_interpreter_state_create

#pragma region une_interpreter_state_free
void une_interpreter_state_free(une_interpreter_state is)
{
  return; // FIXME:
}
#pragma endregion une_interpreter_state_free
