/*
parser_state.c - Une
Modified 2021-05-22
*/

#include "parser_state.h"

#pragma region une_parser_state_create
une_parser_state une_parser_state_create(void)
{
  return (une_parser_state){
    .index = 0,
    .inside_function = false,
    .inside_loop = false,
    .tokens = NULL
  };
}
#pragma endregion une_parser_state_create

#pragma region une_parser_state_free
void une_parser_state_free(une_parser_state ps)
{
  une_tokens_free(ps.tokens);
}
#pragma endregion une_parser_state_free
