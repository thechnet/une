/*
token.h - Une
Updated 2021-05-21
*/

#ifndef UNE_TOKEN_H
#define UNE_TOKEN_H

#include "../primitive.h"
#include "../tools.h"

#pragma region une_token_type
typedef enum _une_token_type {
  __UNE_TT_none__,
  UNE_TT_INT,
  UNE_TT_FLT,
  UNE_TT_ID,
  UNE_TT_STR,
  UNE_TT_LPAR,
  UNE_TT_RPAR,
  UNE_TT_LBRC,
  UNE_TT_RBRC,
  UNE_TT_LSQB,
  UNE_TT_RSQB,
  UNE_TT_SEP,
  UNE_TT_NEW,
  UNE_TT_EOF,
  UNE_TT_SET,
  UNE_TT_NOT,
  UNE_TT_AND, // Begin And/Or Tokens
  UNE_TT_OR, // End And/Or Tokens
  UNE_TT_EQU, // Begin Condition Tokens
  UNE_TT_NEQ,
  UNE_TT_GTR,
  UNE_TT_GEQ,
  UNE_TT_LSS,
  UNE_TT_LEQ, // End Condition Tokens
  UNE_TT_ADD, // Begin Add/Sub Tokens
  UNE_TT_SUB, // End Add/Sub Tokens
  UNE_TT_MUL, // Begin Term Tokens
  UNE_TT_DIV,
  UNE_TT_FDIV,
  UNE_TT_MOD,  // End Term Tokens
  UNE_TT_POW,
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
  __UNE_TT_max__
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

#ifdef UNE_DEBUG
wchar_t *une_token_to_wcs(une_token token);
void une_tokens_display(une_token *tokens);
#endif /* UNE_DEBUG */

void une_token_free(une_token token);
void une_tokens_free(une_token *tokens);

#endif /* !UNE_TOKEN_H */
