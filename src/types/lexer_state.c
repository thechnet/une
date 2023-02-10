/*
lexer_state.c - Une
Modified 2023-02-10
*/

/* Header-specific includes. */
#include "lexer_state.h"

/*
Initialize a une_lexer_state struct.
*/
une_lexer_state une_lexer_state_create(bool read_from_file, char *path, wchar_t *text)
{
  return (une_lexer_state){
    .read_from_file = read_from_file,
    .path = path,
    .text = text,
    .in = (une_istream){ 0 },
    .pull = NULL,
    .peek = NULL,
    .now = NULL,
    .in_str_expression = false,
    .begin_str_expression = false
  };
}
