/*
lexer_state.c - Une
Modified 2021-05-22
*/

#include "lexer_state.h"

#pragma region une_lexer_state_create
une_lexer_state une_lexer_state_create(void)
{
  return (une_lexer_state){
    .read_from_wcs = false,
    .file = NULL,
    .wcs = NULL,
    .index = 0
  };
}
#pragma endregion une_lexer_state_create

#pragma region une_lexer_state_free
void une_lexer_state_free(une_lexer_state ls)
{
  if (ls.wcs != NULL) free(ls.wcs);
}
#pragma endregion une_lexer_state_free
