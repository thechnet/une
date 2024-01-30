/*
token.h - Une
Modified 2024-01-30
*/

#ifndef UNE_TOKEN_H
#define UNE_TOKEN_H

/* Header-specific includes. */
#include "../common.h"

/*
Kind of une_token.
*/
typedef enum une_token_kind_ {
	UNE_TK_none__, /* Undefined token kind. */
	UNE_TK_INT,
	UNE_TK_FLT,
	UNE_TK_STR,
	UNE_TK_STR_EXPRESSION_BEGIN,
	UNE_TK_STR_EXPRESSION_END,
	UNE_TK_NAME,
	UNE_TK_NATIVE,
	UNE_TK_EOF,
	#define UNE_R_BGN_KEYWORD_TOKENS UNE_TK_TRUE
	UNE_TK_TRUE,
	UNE_TK_FALSE,
	UNE_TK_VOID,
	UNE_TK_THIS,
	UNE_TK_ANY,
	UNE_TK_ALL,
	UNE_TK_COVER,
	UNE_TK_GLOBAL,
	UNE_TK_IF,
	UNE_TK_ELIF,
	UNE_TK_ELSE,
	UNE_TK_FOR,
	UNE_TK_FROM,
	UNE_TK_TILL,
	UNE_TK_IN,
	UNE_TK_WHILE,
	UNE_TK_CONTINUE,
	UNE_TK_BREAK,
	UNE_TK_RETURN,
	UNE_TK_EXIT,
	UNE_TK_ASSERT,
	#define UNE_R_END_KEYWORD_TOKENS UNE_TK_ASSERT
	#define UNE_R_BGN_OPERATOR_TOKENS UNE_TK_LPAR
	UNE_TK_LPAR,
	UNE_TK_RPAR,
	UNE_TK_LBRC,
	UNE_TK_RBRC,
	UNE_TK_LSQB,
	UNE_TK_RSQB,
	UNE_TK_SEP,
	UNE_TK_NEW,
	#define UNE_R_BGN_CONDITION_TOKENS UNE_TK_EQU
	UNE_TK_EQU,
	UNE_TK_NEQ,
	UNE_TK_GEQ,
	UNE_TK_GTR,
	UNE_TK_LEQ,
	UNE_TK_LSS,
	#define UNE_R_END_CONDITION_TOKENS UNE_TK_LSS
	UNE_TK_NOT,
	#define UNE_R_BGN_AND_OR_TOKENS UNE_TK_AND
	UNE_TK_AND,
	UNE_TK_OR,
	UNE_TK_NULLISH,
	#define UNE_R_END_AND_OR_TOKENS UNE_TK_NULLISH
	#define UNE_R_BGN_ASSIGNMENT_TOKENS UNE_TK_SET
	UNE_TK_SET,
	UNE_TK_PLUSEQUAL,
	UNE_TK_MINUSEQUAL,
	UNE_TK_STARSTAREQUAL,
	UNE_TK_STAREQUAL,
	UNE_TK_SLASHSLASHEQUAL,
	UNE_TK_SLASHEQUAL,
	UNE_TK_PERCENTEQUAL,
	#define UNE_R_END_ASSIGNMENT_TOKENS UNE_TK_PERCENTEQUAL
	UNE_TK_RIGHTARROW,
	#define UNE_R_BGN_ADD_SUB_TOKENS UNE_TK_ADD
	UNE_TK_ADD,
	UNE_TK_SUB,
	#define UNE_R_END_ADD_SUB_TOKENS UNE_TK_SUB
	UNE_TK_POW,
	#define UNE_R_BGN_TERM_TOKENS UNE_TK_MUL
	UNE_TK_MUL,
	UNE_TK_FDIV,
	UNE_TK_DIV,
	UNE_TK_MOD,
	#define UNE_R_END_TERM_TOKENS UNE_TK_MOD
	UNE_TK_QMARK,
	UNE_TK_COLON,
	UNE_TK_DOTDOT,
	UNE_TK_DOT,
	#define UNE_R_END_OPERATOR_TOKENS UNE_TK_DOT
	UNE_TK_max__,
} une_token_kind;

/*
Holds information about a lexical token.
*/
typedef struct une_token_ {
	une_token_kind kind;
	une_position pos;
	une_value value;
} une_token;

/*
*** Interface.
*/

/*
Condition to check whether a une_token_kind is valid.
*/
#define UNE_TOKEN_KIND_IS_VALID(kind)\
	(kind > UNE_TK_none__ && kind < UNE_TK_max__)

extern const wchar_t *une_token_table[];

une_token une_token_create(une_token_kind kind);
void une_token_free(une_token token);
void une_tokens_free(une_token *tokens);

#ifdef UNE_DEBUG
une_static__ const wchar_t *une_token_kind_to_wcs(une_token_kind kind);
une_static__ wchar_t *une_token_to_wcs(une_token token);
void une_tokens_display(une_token *tokens);
#endif /* UNE_DEBUG */

#endif /* !UNE_TOKEN_H */
