/*
lexer.h - Une
Updated 2021-04-28
*/

#ifndef UNE_LEXER_H
#define UNE_LEXER_H

#include "primitive.h"
#include "tools.h"
#include "types/token.h"
#include "types/error.h"
#include <string.h>

une_token *une_lex_wcs(wchar_t *text, une_error *error);

#ifdef UNE_DEBUG
une_token *une_lex_file(char *path, une_error *error);
#endif /* UNE_DEBUG */

#endif /* !UNE_LEXER_H */
