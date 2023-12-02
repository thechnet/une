/*
lexer_state.h - Une
Modified 2023-11-17
*/

#ifndef UNE_LEXER_STATE_H
#define UNE_LEXER_STATE_H

/* Header-specific includes. */
#include "../primitive.h"
#include "token.h"

/*
Holds the state of the lexer.
*/
typedef struct une_lexer_state_ {
	wchar_t *text;
	size_t text_length;
	size_t text_index;
	une_token *tokens;
	size_t tokens_size;
	size_t tokens_count;
	bool in_str_expression;
	bool begin_str_expression;
	size_t line;
} une_lexer_state;

/*
*** Interface.
*/

une_lexer_state une_lexer_state_create(void);

#endif /* UNE_LEXER_STATE_H */
