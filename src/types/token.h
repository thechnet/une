/*
token.h - Une
Updated 2021-04-24
*/

#ifndef UNE_TOKEN_H
#define UNE_TOKEN_H

#include "../primitive.h"

#pragma region une_token_type
typedef enum _une_token_type {
  // Data
  UNE_TT_INT,
  UNE_TT_FLT,
  UNE_TT_ID,
  UNE_TT_STR,
  // Grouping and Ordering
  UNE_TT_LPAR,
  UNE_TT_RPAR,
  UNE_TT_LBRC,
  UNE_TT_RBRC,
  UNE_TT_LSQB,
  UNE_TT_RSQB,
  UNE_TT_SEP,
  UNE_TT_NEW,
  UNE_TT_EOF,
  // Operators
  UNE_TT_SET,
  UNE_TT_ADD,
  UNE_TT_SUB,
  UNE_TT_MUL,
  UNE_TT_DIV,
  UNE_TT_FDIV,
  UNE_TT_MOD,
  UNE_TT_POW,
  // Comparisons
  UNE_TT_NOT,
  UNE_TT_AND,
  UNE_TT_OR,
  UNE_TT_EQU,
  UNE_TT_NEQ,
  UNE_TT_GTR,
  UNE_TT_GEQ,
  UNE_TT_LSS,
  UNE_TT_LEQ,
  // Clause Keywords
  UNE_TT_IF,
  UNE_TT_ELIF,
  UNE_TT_ELSE,
  UNE_TT_FOR,
  UNE_TT_FROM,
  UNE_TT_TILL,
  UNE_TT_WHILE,
  UNE_TT_DEF,
  UNE_TT_CONTINUE,
  UNE_TT_BREAK,
  UNE_TT_RETURN,
} une_token_type;
#pragma endregion une_token_type

#pragma region une_token
typedef struct _une_token {
  une_token_type type;
  une_position pos;
  une_value value;
} une_token;
#pragma endregion une_token

const wchar_t *une_token_type_to_wcs(une_token_type type);
wchar_t *une_token_to_wcs(une_token token);
void une_tokens_display(une_token *tokens);
void une_tokens_free(une_token *tokens);

#endif /* !UNE_TOKEN_H */
