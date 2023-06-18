/*
token.c - Une
Modified 2023-06-18
*/

/* Header-specific includes. */
#include "token.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "../builtin_functions.h"

/*
Token name table.
*/
const wchar_t *une_token_table[] = {
  L"int",
  L"flt",
  L"str",
  L"\"{",
  L"}\"",
  L"id",
  L"builtin",
  L"EOF",
  L"function",
  /* Begin keyword tokens. */
  L"True",
  L"False",
  L"Void",
  L"this",
  L"cover",
  L"global",
  L"if",
  L"elif",
  L"else",
  L"for",
  L"from",
  L"till",
  L"in",
  L"while",
  L"continue",
  L"break",
  L"return",
  L"exit",
  L"assert",
  /* Begin operator tokens. */
  L"(",
  L")",
  L"{",
  L"}",
  L"[",
  L"]",
  L",",
  L";",
  L"==",
  L"!=",
  L">=",
  L">",
  L"<=",
  L"<",
  L"!",
  L"&&",
  L"||",
  L"??",
  L"=",
  L"+=",
  L"-=",
  L"**=",
  L"*=",
  L"//=",
  L"/=",
  L"%=",
  L"+",
  L"-",
  L"**",
  L"*",
  L"//",
  L"/",
  L"%",
  L"?",
  L":",
  L"..",
  L".",
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
  assert(UNE_TOKEN_TYPE_IS_VALID(token.type));
  
  /* Free members. */
  switch (token.type) {
    case UNE_TT_ID:
    case UNE_TT_STR:
      free(token.value._wcs);
      break;
    default:
      break;
  }
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
    une_token_type token_type = tokens[i].type;
    une_token_free(tokens[i]);
    if (token_type == UNE_TT_EOF)
      break;
    i++;
  }
  
  /* Free list. */
  free(tokens);
}

/*
Return a text representation of a une_token_type.
*/
une_static__ const wchar_t *une_token_type_to_wcs(une_token_type type)
{
  assert(UNE_TOKEN_TYPE_IS_VALID(type));
  
  return une_token_table[type-1];
}

/*
Return a text representation of a une_token.
This function is not dynamic and will cause a buffer overflow
if the returned array is longer than UNE_SIZE_TOKEN_AS_WCS. This could
realistically happen, were this function used in a real-world
environment, but since it is only used for debugging, I'm
leaving this vulnerability in here.
*/
#ifdef UNE_DEBUG
une_static__ wchar_t *une_token_to_wcs(une_token token)
{
  /* Write token type. */
  wchar_t *str = malloc(UNE_SIZE_TOKEN_AS_WCS*sizeof(*str));
  verify(str);
  int str_len = 0;
  str_len += swprintf(str, UNE_SIZE_TOKEN_AS_WCS, UNE_COLOR_TOKEN_TYPE);
  
  assert(UNE_TOKEN_TYPE_IS_VALID(token.type));
  str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L"%ls", une_token_type_to_wcs(token.type));
  str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, RESET);
  
  /* Write token value if it exists. */
  switch (token.type) {
    
    /* Numbers. */
    case UNE_TT_INT:
      str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE UNE_PRINTF_UNE_INT RESET, token.value._int);
      break;
    case UNE_TT_FLT:
      str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE UNE_PRINTF_UNE_FLT RESET, token.value._flt);
      break;
    
    /* Strings. */
    case UNE_TT_STR:
      str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE L"\"%ls" UNE_COLOR_TOKEN_VALUE L"\"" RESET, token.value._wcs);
      break;
    case UNE_TT_ID:
      str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE L"%ls" RESET, token.value._wcs);
      break;
    
    /* Built-in function. */
    case UNE_TT_BUILTIN:
      str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE L"%ls" RESET, une_builtin_function_to_wcs((une_builtin_function)token.value._int));
      break;
    
  }
  
  assert(str_len < UNE_SIZE_TOKEN_AS_WCS);
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
    free(token_as_wcs);
    if (tokens[i].type == UNE_TT_EOF)
      break;
    i++;
  }
}
#endif /* UNE_DEBUG */
