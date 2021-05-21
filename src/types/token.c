/*
token.c - Une
Updated 2021-05-21
*/

#include "token.h"

#pragma region Token Name Table
wchar_t *une_token_table[] = {
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
  L"return"
};
#pragma endregion Token Name Table

#pragma region une_token_type_to_wcs
const wchar_t *une_token_type_to_wcs(une_token_type type)
{
  if (type <= 0 || type >= __UNE_TT_max__) WERR(L"Token type out of bounds: %d", type);
  return une_token_table[type-1];
}
#pragma endregion une_token_type_to_wcs

#pragma region une_token_to_wcs
#ifdef UNE_DEBUG
/*
This function is not dynamic and will cause a buffer overflow
if the returned array is longer than UNE_SIZE_MEDIUM. This could
realistically happen, were this function used in a real-world
environment, but since it is only used for debugging, I'm
leaving this vulnerability in here.
*/
wchar_t *une_token_to_wcs(une_token token)
{
  wchar_t *str = rmalloc(UNE_SIZE_MEDIUM * sizeof(*str));
  wcscpy(str, UNE_COLOR_TOKEN_TYPE);
  wcscat(str, une_token_type_to_wcs(token.type));
  wcscat(str, UNE_COLOR_NEUTRAL);
  switch (token.type) {
    case UNE_TT_INT:
      swprintf(str+wcslen(str), UNE_SIZE_MEDIUM,
               L":" UNE_COLOR_TOKEN_VALUE L"%lld" UNE_COLOR_NEUTRAL,
               token.value._int);
      break;
    
    case UNE_TT_FLT:
      swprintf(str+wcslen(str), UNE_SIZE_MEDIUM,
               L":" UNE_COLOR_TOKEN_VALUE L"%.3f" UNE_COLOR_NEUTRAL,
               token.value._flt);
      break;
    
    case UNE_TT_STR:
      swprintf(str+wcslen(str), UNE_SIZE_MEDIUM,
               L":" UNE_COLOR_TOKEN_VALUE L"\"%ls" UNE_COLOR_TOKEN_VALUE L"\""
               UNE_COLOR_NEUTRAL,
               token.value._wcs);
      break;
    
    case UNE_TT_ID:
      swprintf(str+wcslen(str), UNE_SIZE_MEDIUM,
               L":" UNE_COLOR_TOKEN_VALUE L"%ls" UNE_COLOR_NEUTRAL,
               token.value._wcs);
      break;
  }
  return str;
}
#endif /* UNE_DEBUG */
#pragma endregion une_token_to_wcs

#pragma region une_tokens_display
#ifdef UNE_DEBUG
void une_tokens_display(une_token *tokens)
{
  size_t i = 0;
  wchar_t *token_as_wcs;
  while (true) {
    token_as_wcs = une_token_to_wcs(tokens[i]);
    wprintf(L"%ls ", token_as_wcs);
    free(token_as_wcs);
    if (tokens[i].type == UNE_TT_EOF) break;
    i++;
  }
  wprintf(L"\n");
}
#endif /* UNE_DEBUG */
#pragma endregion une_tokens_display

#pragma region une_token_free
void une_token_free(une_token token)
{
  if (
    token.type <= __UNE_TT_none__ ||
    token.type >= __UNE_TT_max__
  ) {
    WERR(L"token.type=%d", (int)token.type)
  }
  
  switch (token.type) {
    case UNE_TT_ID:
    case UNE_TT_STR:
      free(token.value._wcs);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
      wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Token: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_token_type_to_wcs(token.type));
      #endif
      break;
    
    #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
    default:
      wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Token: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_token_type_to_wcs(token.type));
    #endif
  }
}
#pragma endregion une_token_free

#pragma region une_tokens_free
void une_tokens_free(une_token *tokens)
{
  if (tokens == NULL) return;
  size_t i = 0;
  while (true) {
    une_token_free(tokens[i]);
    if (tokens[i].type == UNE_TT_EOF) break;
    i++;
  }
  free(tokens);
}
#pragma endregion une_tokens_free
