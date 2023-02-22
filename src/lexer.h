/*
lexer.h - Une
Modified 2023-02-22
*/

#ifndef UNE_LEXER_H
#define UNE_LEXER_H

/* Header-specific includes. */
#include "primitive.h"
#include "types/error.h"
#include "types/lexer_state.h"
#include "types/token.h"
#include "tools.h"

/*
*** Interface.
*/

/*
Condition to check whether a character can be in a number.
*/
#define UNE_LEXER_WC_IS_DIGIT(wc) (wc >= L'0' && wc <= L'9')

/*
Condition to check whether a character can be the first character of an id.
*/
#define UNE_LEXER_WC_CAN_BEGIN_ID(wc) ((wc >= L'a' && wc <= L'z') || (wc >= L'A' && wc <= L'Z') || wc == L'_')

/*
Condition to check whether a character can be in an id.
*/
#define UNE_LEXER_WC_CAN_BE_IN_ID(wc) (UNE_LEXER_WC_CAN_BEGIN_ID(wc) || UNE_LEXER_WC_IS_DIGIT(wc))

/*
Condition to check whether a character is whitespace that can always be ignored.
*/
#define UNE_LEXER_WC_IS_SOFT_WHITESPACE(wc) (wc == L' ' || wc == L'\t')

/*
Condition to check whether a character is legal but non-printable.
*/
#define UNE_LEXER_WC_IS_INVISIBLE(wc) (wc == L'\r')

/*
Condition to check whether a character can seperate statements.
*/
#define UNE_LEXER_WC_IS_HARD_WHITESPACE(wc) (wc == L'\r' || wc == L'\n' || wc == L';')

/*
Condition to check if a character is any kind of whitespace.
*/
#define UNE_LEXER_WC_IS_WHITESPACE(wc) (UNE_LEXER_WC_IS_SOFT_WHITESPACE(wc) || UNE_LEXER_WC_IS_HARD_WHITESPACE(wc))

/*
Lexer function template.
*/
#define une_lexer__(id__) une_static__ une_token (id__)(une_error *error, une_lexer_state *ls)

une_token *une_lex(une_error*, une_lexer_state*);

une_lexer__(une_lex_operator);
une_lexer__(une_lex_num);
une_lexer__(une_lex_str);
une_lexer__(une_lex_keyword_or_identifier);

#endif /* !UNE_LEXER_H */
