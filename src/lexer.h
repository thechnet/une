/*
lexer.h - Une
Updated 2021-05-22
*/

#ifndef UNE_LEXER_H
#define UNE_LEXER_H

#include "primitive.h"
#include "tools.h"
#include "types/token.h"
#include "types/instance.h"
#include <string.h>

// Public Lexer Interface.
une_token *une_lex_wcs(une_instance *inst);

#ifdef UNE_DEBUG
une_token *une_lex_file(char *path, une_error *error);
#endif /* UNE_DEBUG */

#endif /* !UNE_LEXER_H */
