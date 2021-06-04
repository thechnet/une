/*
lexer.h - Une
Updated 2021-06-04
*/

#ifndef UNE_LEXER_H
#define UNE_LEXER_H

#include "primitive.h"
#include "types/error.h"
#include "types/lexer_state.h"
#include "tools.h"
#include "types/token.h"
#include <string.h>

// Public Lexer Interface.
une_token* une_lex(une_error*, une_lexer_state*);

// Lexer Functions
une_token une_lex_num(une_error *error, une_lexer_state *ls);
une_token une_lex_str(une_error *error, une_lexer_state *ls);
une_token une_lex_id (une_error *error, une_lexer_state *ls);

// Character Stream
void une_lexer_fgetwc(une_lexer_state *ls);
void une_lexer_sgetwc(une_lexer_state *ls);
wint_t une_lexer_fpeekwc(une_lexer_state *ls);
wint_t une_lexer_speekwc(une_lexer_state *ls);

#endif /* !UNE_LEXER_H */
