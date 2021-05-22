/*
interpreter_state.c - Une
Modified 2021-05-22
*/

#include "interpreter_state.h"

#pragma region une_interpreter_state_create
une_interpreter_state une_interpreter_state_create(void)
{
  return (une_interpreter_state){
    .context = une_context_create(),
    .should_return = false
  };
}
#pragma endregion une_interpreter_state_create

#pragma region une_interpreter_state_free
void une_interpreter_state_free(une_interpreter_state is)
{
  une_context_free(is.context);
}
#pragma endregion une_interpreter_state_free
