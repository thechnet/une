/*
lexer.h - Une
Updated 2021-04-17
*/

#ifndef UNE_LEXER_H
#define UNE_LEXER_H

#include "primitive.h"
#include "types/token.h"
#include "types/error.h"
#include <string.h>
#include "tools.h"

une_token *une_lex_wcs(wchar_t *text, une_error *error);

#endif /* !UNE_LEXER_H */