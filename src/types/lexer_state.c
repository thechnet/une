/*
lexer_state.c - Une
Modified 2023-11-17
*/

/* Header-specific includes. */
#include "lexer_state.h"

/*
Initialize a une_lexer_state struct.
*/
une_lexer_state une_lexer_state_create(void)
{
	return (une_lexer_state){
		.text = NULL,
		.text_length = 0,
		.text_index = 0,
		.tokens = NULL,
		.tokens_count = 0,
		.tokens_size = UNE_SIZE_TOKEN_BUF,
		.in_str_expression = false,
		.begin_str_expression = false,
		.line = 1,
	};
}
