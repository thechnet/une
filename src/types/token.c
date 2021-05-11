/*
token.c - Une
Updated 2021-05-10
*/

#include "token.h"

#pragma region une_token_type_to_wcs
const wchar_t *une_token_type_to_wcs(une_token_type type)
{
  switch (type) {
    // Data
    case UNE_TT_INT: return L"INT";
    case UNE_TT_FLT: return L"FLT";
    case UNE_TT_ID: return L"ID";
    case UNE_TT_STR: return L"STR";
    // Grouping and Ordering
    case UNE_TT_LPAR: return L"(";
    case UNE_TT_RPAR: return L")";
    case UNE_TT_LBRC: return L"{";
    case UNE_TT_RBRC: return L"}";
    case UNE_TT_LSQB: return L"[";
    case UNE_TT_RSQB: return L"]";
    case UNE_TT_SEP: return L",";
    case UNE_TT_NEW: return L";";
    case UNE_TT_EOF: return L"EOF";
    // Operators
    case UNE_TT_SET: return L"=";
    case UNE_TT_ADD: return L"+";
    case UNE_TT_SUB: return L"-";
    case UNE_TT_MUL: return L"*";
    case UNE_TT_DIV: return L"/";
    case UNE_TT_FDIV: return L"//";
    case UNE_TT_MOD: return L"%";
    case UNE_TT_POW: return L"**";
    // Comparisons
    case UNE_TT_EQU: return L"==";
    case UNE_TT_NEQ: return L"!=";
    case UNE_TT_GTR: return L">";
    case UNE_TT_GEQ: return L">=";
    case UNE_TT_LSS: return L"<";
    case UNE_TT_LEQ: return L"<=";
    case UNE_TT_NOT: return L"not";
    case UNE_TT_AND: return L"and";
    case UNE_TT_OR: return L"or";
    // Clause Keywords
    case UNE_TT_IF: return L"if";
    case UNE_TT_ELIF: return L"elif";
    case UNE_TT_ELSE: return L"else";
    case UNE_TT_FOR: return L"for";
    case UNE_TT_FROM: return L"from";
    case UNE_TT_TILL: return L"till";
    case UNE_TT_WHILE: return L"while";
    case UNE_TT_DEF: return L"def";
    case UNE_TT_RETURN: return L"return";
    case UNE_TT_BREAK: return L"break";
    case UNE_TT_CONTINUE: return L"continue";

    default: WERR(L"Unknown token type.");
  }
}

// const wchar_t *une_token_type_to_wcs(une_token_type type)
// {
//   switch (type) {
//     // Data
//     case UNE_TT_INT: return L"INT";
//     case UNE_TT_FLT: return L"FLT";
//     case UNE_TT_ID: return L"ID";
//     case UNE_TT_STR: return L"STR";
//     // Grouping and Ordering
//     case UNE_TT_LPAR: return L"LPAR";
//     case UNE_TT_RPAR: return L"RPAR";
//     case UNE_TT_LBRC: return L"LBRC";
//     case UNE_TT_RBRC: return L"RBRC";
//     case UNE_TT_LSQB: return L"LSQB";
//     case UNE_TT_RSQB: return L"RSQB";
//     case UNE_TT_SEP: return L"SEP";
//     case UNE_TT_NEW: return L"NEW";
//     case UNE_TT_EOF: return L"EOF";
//     // Operators
//     case UNE_TT_SET: return L"SET";
//     case UNE_TT_ADD: return L"ADD";
//     case UNE_TT_SUB: return L"SUB";
//     case UNE_TT_MUL: return L"MUL";
//     case UNE_TT_DIV: return L"DIV";
//     case UNE_TT_FDIV: return L"FDIV";
//     case UNE_TT_MOD: return L"MOD";
//     case UNE_TT_POW: return L"POW";
//     // Comparisons
//     case UNE_TT_EQU: return L"EQU";
//     case UNE_TT_NEQ: return L"NEQ";
//     case UNE_TT_GTR: return L"GTR";
//     case UNE_TT_GEQ: return L"GEQ";
//     case UNE_TT_LSS: return L"LSS";
//     case UNE_TT_LEQ: return L"LEQ";
//     case UNE_TT_NOT: return L"NOT";
//     case UNE_TT_AND: return L"AND";
//     case UNE_TT_OR: return L"OR";
//     // Clause Keywords
//     case UNE_TT_IF: return L"IF";
//     case UNE_TT_ELIF: return L"ELIF";
//     case UNE_TT_ELSE: return L"ELSE";
//     case UNE_TT_FOR: return L"FOR";
//     case UNE_TT_FROM: return L"FROM";
//     case UNE_TT_TILL: return L"TILL";
//     case UNE_TT_WHILE: return L"WHILE";
//     case UNE_TT_DEF: return L"DEF";
//     case UNE_TT_RETURN: return L"RETURN";
//     case UNE_TT_BREAK: return L"BREAK";
//     case UNE_TT_CONTINUE: return L"CONTINUE";

//     default: WERR(L"Unknown token type.");
//   }
// }
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
               L":" UNE_COLOR_TOKEN_VALUE L"\"%ls" UNE_COLOR_TOKEN_VALUE L"\"" UNE_COLOR_NEUTRAL,
               token.value._wcs);
      break;
    
    case UNE_TT_ID:
      swprintf(str+wcslen(str), UNE_SIZE_MEDIUM,
               L":" UNE_COLOR_TOKEN_VALUE L"%ls" UNE_COLOR_NEUTRAL,
               token.value._wcs);
      break;
    
    // FIXME: ADD MISSING CASES HERE
    default:
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
  switch (token.type) {
    case UNE_TT_ID:
    case UNE_TT_STR:
      free(token.value._wcs);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Token: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_token_type_to_wcs(token.type));
      #endif
      break;
    
    case UNE_TT_INT:
    case UNE_TT_FLT:
    case UNE_TT_LPAR:
    case UNE_TT_RPAR:
    case UNE_TT_LBRC:
    case UNE_TT_RBRC:
    case UNE_TT_LSQB:
    case UNE_TT_RSQB:
    case UNE_TT_SEP:
    case UNE_TT_NEW:
    case UNE_TT_EOF:
    case UNE_TT_SET:
    case UNE_TT_ADD:
    case UNE_TT_SUB:
    case UNE_TT_MUL:
    case UNE_TT_DIV:
    case UNE_TT_FDIV:
    case UNE_TT_MOD:
    case UNE_TT_POW:
    case UNE_TT_NOT:
    case UNE_TT_AND:
    case UNE_TT_OR:
    case UNE_TT_EQU:
    case UNE_TT_NEQ:
    case UNE_TT_GTR:
    case UNE_TT_GEQ:
    case UNE_TT_LSS:
    case UNE_TT_LEQ:
    case UNE_TT_IF:
    case UNE_TT_ELIF:
    case UNE_TT_ELSE:
    case UNE_TT_FOR:
    case UNE_TT_FROM:
    case UNE_TT_TILL:
    case UNE_TT_WHILE:
    case UNE_TT_DEF:
    case UNE_TT_CONTINUE:
    case UNE_TT_BREAK:
    case UNE_TT_RETURN:
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Token: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_token_type_to_wcs(token.type));
      #endif
      break;
    
    default:
      WERR(L"Unhandled token type in une_token_free()!\n");
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
