/*
parser_state.c - Une
Modified 2021-06-13
*/

/* Header-specific includes. */
#include "parser_state.h"

/*
Initialize a une_parser_state struct.
*/
une_parser_state une_parser_state_create(void)
{
  return (une_parser_state){
    .inside_loop = false,
    .in = (une_istream){ 0 },
    .pull = NULL,
    .peek = NULL,
    .now = NULL
  };
}
