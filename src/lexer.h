/*
lexer.h - Une
Updated 2021-05-24
*/

#ifndef UNE_LEXER_H
#define UNE_LEXER_H

#include "primitive.h"
#include "tools.h"
#include "types/token.h"
#include "types/instance.h"
#include <string.h>

// Public Lexer Interface.
void une_lex(une_instance *inst);

// Lexer Functions
une_token une_lex_num(une_instance *inst);
une_token une_lex_str(une_instance *inst);
une_token une_lex_id (une_instance *inst);

// Character Stream
size_t une_lexer_index(une_lexer_state *ls);
void une_lexer_fgetwc(une_lexer_state *ls);
void une_lexer_sgetwc(une_lexer_state *ls);
wint_t une_lexer_fpeekwc(une_lexer_state *ls);
wint_t une_lexer_speekwc(une_lexer_state *ls);

#endif /* !UNE_LEXER_H */
