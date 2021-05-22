/*
instance.c - Une
Modified 2021-05-22
*/

#include "error.h"
#include "lexer_state.h"
#include "parser_state.h"
#include "interpreter_state.h"
#include "instance.h"

#pragma region une_instance_create
une_instance une_instance_create(void)
{
  return (une_instance){
    .error = une_error_create(),
    .ls = une_lexer_state_create(),
    .ps = une_parser_state_create(),
    .is = une_interpreter_state_create()
  };
}
#pragma endregion une_instance_create

#pragma region une_instance_free
void une_instance_free(une_instance instance)
{
  une_error_free(instance.error);
  une_lexer_state_free(instance.ls);
  une_parser_state_free(instance.ps);
  une_interpreter_state_free(instance.is);
}
#pragma endregion une_instance_free
