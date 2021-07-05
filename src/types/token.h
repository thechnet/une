/*
token.h - Une
Modified 2021-07-05
*/

#ifndef UNE_TOKEN_H
#define UNE_TOKEN_H

/* Header-specific includes. */
#include "../primitive.h"

/*
Type of une_token.
*/
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
  #define UNE_R_BGN_AND_OR_TOKENS UNE_TT_AND
  UNE_TT_AND,
  UNE_TT_OR,
  #define UNE_R_END_AND_OR_TOKENS UNE_TT_OR
  #define UNE_R_BGN_CONDITION_TOKENS UNE_TT_EQU
  UNE_TT_EQU,
  UNE_TT_NEQ,
  UNE_TT_GTR,
  UNE_TT_GEQ,
  UNE_TT_LSS,
  UNE_TT_LEQ,
  #define UNE_R_END_CONDITION_TOKENS UNE_TT_LEQ
  #define UNE_R_BGN_ADD_SUB_TOKENS UNE_TT_ADD
  UNE_TT_ADD,
  UNE_TT_SUB,
  #define UNE_R_END_ADD_SUB_TOKENS UNE_TT_SUB
  #define UNE_R_BGN_TERM_TOKENS UNE_TT_MUL
  UNE_TT_MUL,
  UNE_TT_DIV,
  UNE_TT_FDIV,
  UNE_TT_MOD,
  #define UNE_R_END_TERM_TOKENS UNE_TT_MOD
  #define UNE_R_BGN_POWER_TOKENS UNE_TT_POW
  UNE_TT_POW,
  #define UNE_R_END_POWER_TOKENS UNE_TT_POW
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
  UNE_TT_GLOBAL,
  UNE_TT_QMARK,
  UNE_TT_COLON,
  __UNE_TT_max__,
} une_token_type;

/*
Holds information about a lexical token.
*/
typedef struct _une_token {
  une_token_type type;
  une_position pos;
  une_value value;
} une_token;

/*
*** Interface.
*/

/*
Condition to check whether a une_token_type is valid.
*/
#define UNE_TOKEN_TYPE_IS_VALID(type)\
  (type > __UNE_TT_none__ && type < __UNE_TT_max__)

/*
Verify une_token_type.
*/
#define UNE_VERIFY_TOKEN_TYPE(type)\
  if (!UNE_TOKEN_TYPE_IS_VALID(type))\
    ERR(L"Invalid une_token_type %lld.", type);

une_token une_token_create(une_token_type type);
void une_token_free(une_token token);
void une_tokens_free(une_token *tokens);

#ifdef UNE_DEBUG
/*__une_static*/ const wchar_t *une_token_type_to_wcs(une_token_type type);
#endif /* UNE_DEBUG */

#ifdef UNE_DEBUG
/*__une_static*/ wchar_t *une_token_to_wcs(une_token token);
#endif /* UNE_DEBUG */

#ifdef UNE_DEBUG
void une_tokens_display(une_token *tokens);
#endif /* UNE_DEBUG */

#endif /* !UNE_TOKEN_H */
