/*
lexer_state.c - Une
Modified 2021-06-04
*/

#include "lexer_state.h"

#pragma region une_lexer_state_create
une_lexer_state une_lexer_state_create(bool read_from_file, char* path, wchar_t* text)
{
  return (une_lexer_state){
    .read_from_file = read_from_file,
    .path = path,
    .file = NULL,
    .text = text,
    .index = 0,
    .wc = WEOF,
    .get = NULL,
    .peek = NULL
  };
}
#pragma endregion une_lexer_state_create

#pragma region une_lexer_state_free
void une_lexer_state_free(une_lexer_state ls)
{
  return; // FIXME:
}
#pragma endregion une_lexer_state_free
