/*
lexer.h - Une
Modified 2023-07-02
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
Character groups.
*/
#define UNE_LEXER_WC_IS_LOWERCASE_LETTER(wc) (wc >= L'a' && wc <= L'z')
#define UNE_LEXER_WC_IS_UPPERCASE_LETTER(wc) (wc >= L'A' && wc <= L'Z')
#define UNE_LEXER_WC_IS_LETTER(wc) (UNE_LEXER_WC_IS_LOWERCASE_LETTER(wc) || UNE_LEXER_WC_IS_UPPERCASE_LETTER(wc))
#define UNE_LEXER_WC_IS_DIGIT(wc) (wc >= L'0' && wc <= L'9')
#define UNE_LEXER_WC_IS_LETTERLIKE(wc) (UNE_LEXER_WC_IS_LETTER(wc) || wc == L'_')

/*
Condition to check whether a character can be the first character of an id.
*/
#define UNE_LEXER_WC_CAN_BEGIN_ID(wc) UNE_LEXER_WC_IS_LETTERLIKE(wc)

/*
Condition to check whether a character can be in an id.
*/
#define UNE_LEXER_WC_CAN_BE_IN_ID(wc) (UNE_LEXER_WC_CAN_BEGIN_ID(wc) || UNE_LEXER_WC_IS_DIGIT(wc))

/*
Condition to check whether a character is whitespace that can always be ignored.
*/
#define UNE_LEXER_WC_IS_SOFT_WHITESPACE(wc) (wc == L' ' || wc == L'\t')

/*
Condition to check whether a character is legal but not printable.
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

void une_lex(une_error *error, une_lexer_state *ls);

une_lexer__(une_lex_operator);
une_lexer__(une_lex_number);
une_lexer__(une_lex_string);
une_lexer__(une_lex_keyword_or_identifier);

bool une_lex_number_base(une_error *error, une_lexer_state *ls, int *base);
bool une_lex_number_integer(une_error *error, une_lexer_state *ls, int base, une_int *integer, bool allow_signed, bool allow_e);
bool une_lex_number_fractional_part(une_error *error, une_lexer_state *ls, int base, une_flt *floating);
bool une_lex_number_exponent(une_error *error, une_lexer_state *ls, une_int *exponent);

wchar_t une_lexer_now(une_lexer_state *ls);
wchar_t une_lexer_advance(une_lexer_state *ls);
wchar_t une_lexer_peek(une_lexer_state *ls, une_int offset);
void une_lexer_commit(une_lexer_state *ls, une_token token);
bool une_lexer_digit_to_decimal(wchar_t digit, int *digit_in_decimal);

#endif /* !UNE_LEXER_H */
