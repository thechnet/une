/*
parser_state.c - Une
Modified 2021-06-04
*/

#include "parser_state.h"

#pragma region une_parser_state_create
une_parser_state une_parser_state_create(une_token *tokens)
{
  return (une_parser_state){
    .index = 0,
    .inside_loop = false,
    .tokens = tokens
  };
}
#pragma endregion une_parser_state_create

#pragma region une_parser_state_free
void une_parser_state_free(une_parser_state ps)
{
  return; // FIXME:
}
#pragma endregion une_parser_state_free
