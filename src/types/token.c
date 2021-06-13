/*
token.c - Une
Updated 2021-06-13
*/

/* Header-specific includes. */
#include "token.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Token name table.
*/
const wchar_t *une_token_table[] = {
  L"int",
  L"float",
  L"id",
  L"str",
  L"(",
  L")",
  L"{",
  L"}",
  L"[",
  L"]",
  L",",
  L";",
  L"EOF",
  L"=",
  L"not",
  L"and",
  L"or",
  L"==",
  L"!=",
  L">",
  L">=",
  L"<",
  L"<=",
  L"+",
  L"-",
  L"*",
  L"/",
  L"//",
  L"%",
  L"**",
  L"if",
  L"elif",
  L"else",
  L"for",
  L"from",
  L"till",
  L"while",
  L"def",
  L"continue",
  L"break",
  L"return",
};

/*
Initialize a une_token.
*/
une_token une_token_create(une_token_type type)
{
  return (une_token){
    .type = type,
    .pos = (une_position){ .start = 0, .end = 0},
    .value._vp = NULL
  };
}

/*
Free all members of a une_token.
*/
void une_token_free(une_token token)
{
  /* Ensure valid une_token_type. */
  UNE_VERIFY_TOKEN_TYPE(token.type);
  
  /* Free members. */
  switch (token.type) {
    
    case UNE_TT_ID:
    case UNE_TT_STR:
      une_free(token.value._wcs);
      break;
  
  }
  
  LOGFREE(L"une_token", une_token_type_to_wcs(token.type), token.type);
}

/*
Free a list of une_tokens and each token's members.
*/
void une_tokens_free(une_token *tokens)
{
  /* Ensure list of tokens exists. */
  if (tokens == NULL)
    return;
  
  /* Free each token. */
  size_t i = 0;
  while (true) {
    une_token_free(tokens[i]);
    if (tokens[i].type == UNE_TT_EOF)
      break;
    i++;
  }
  
  /* Free list. */
  une_free(tokens);
}

/*
Return a text representation of a une_token_type.
*/
__une_static const wchar_t *une_token_type_to_wcs(une_token_type type)
{
  /* Ensure une_token_type is valid. */
  UNE_VERIFY_TOKEN_TYPE(type);
  
  return une_token_table[type-1];
}

/*
Return a text representation of a une_token.
This function is not dynamic and will cause a buffer overflow
if the returned array is longer than UNE_SIZE_MEDIUM. This could
realistically happen, were this function used in a real-world
environment, but since it is only used for debugging, I'm
leaving this vulnerability in here.
*/
#ifdef UNE_DEBUG
__une_static wchar_t *une_token_to_wcs(une_token token)
{
  /* Write token type. */
  wchar_t *str = une_malloc(UNE_SIZE_MEDIUM*sizeof(*str));
  wcscpy(str, UNE_COLOR_TOKEN_TYPE);
  wcscat(str, une_token_type_to_wcs(token.type));
  wcscat(str, UNE_COLOR_NEUTRAL);
  
  /* Write token value if it exists. */
  switch (token.type) {
    
    /* Numbers. */
    case UNE_TT_INT:
      swprintf(str+wcslen(str), UNE_SIZE_MEDIUM, L":" UNE_COLOR_TOKEN_VALUE L"%lld" UNE_COLOR_NEUTRAL, token.value._int);
      break;
    case UNE_TT_FLT:
      swprintf(str+wcslen(str), UNE_SIZE_MEDIUM, L":" UNE_COLOR_TOKEN_VALUE L"%.3f" UNE_COLOR_NEUTRAL, token.value._flt);
      break;
    
    /* Strings. */
    case UNE_TT_STR:
      swprintf(str+wcslen(str), UNE_SIZE_MEDIUM, L":" UNE_COLOR_TOKEN_VALUE L"\"%ls" UNE_COLOR_TOKEN_VALUE L"\"" UNE_COLOR_NEUTRAL, token.value._wcs);
      break;
    case UNE_TT_ID:
      swprintf(str+wcslen(str), UNE_SIZE_MEDIUM, L":" UNE_COLOR_TOKEN_VALUE L"%ls" UNE_COLOR_NEUTRAL, token.value._wcs);
      break;
    
  }
  
  return str;
}
#endif /* UNE_DEBUG */

/*
Display the text representations of each item in a list of une_tokens.
*/
#ifdef UNE_DEBUG
void une_tokens_display(une_token *tokens)
{
  /* Ensure list of tokens exists. */
  if (tokens == NULL)
    return;
  
  /* Print the text representation of each token. */
  size_t i = 0;
  wchar_t *token_as_wcs;
  while (true) {
    token_as_wcs = une_token_to_wcs(tokens[i]);
    wprintf(L"%ls ", token_as_wcs);
    une_free(token_as_wcs);
    if (tokens[i].type == UNE_TT_EOF)
      break;
    i++;
  }
}
#endif /* UNE_DEBUG */
