/*
token.c - Une
Modified 2024-01-30
*/

/* Header-specific includes. */
#include "token.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "../natives.h"

/*
Token name table.
*/
const wchar_t *une_token_table[] = {
	L"int",
	L"flt",
	L"str",
	L"\"{",
	L"}\"",
	L"name",
	L"native",
	L"EOF",
	/* Begin keyword tokens. */
	L"True",
	L"False",
	L"Void",
	L"this",
	L"any",
	L"all",
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
	L"->",
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
une_token une_token_create(une_token_kind kind)
{
	return (une_token){
		.kind = kind,
		.pos = (une_position){ .start = 0, .end = 0},
		.value._vp = NULL
	};
}

/*
Free all members of a une_token.
*/
void une_token_free(une_token token)
{
	assert(UNE_TOKEN_KIND_IS_VALID(token.kind));
	
	/* Free members. */
	switch (token.kind) {
		case UNE_TK_NAME:
		case UNE_TK_STR:
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
		une_token_kind token_kind = tokens[i].kind;
		une_token_free(tokens[i]);
		if (token_kind == UNE_TK_EOF)
			break;
		i++;
	}
	
	/* Free list. */
	free(tokens);
}

/*
Return a text representation of a une_token_kind.
*/
une_static__ const wchar_t *une_token_kind_to_wcs(une_token_kind kind)
{
	assert(UNE_TOKEN_KIND_IS_VALID(kind));
	
	return une_token_table[kind-1];
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
	/* Write token kind. */
	wchar_t *str = malloc(UNE_SIZE_TOKEN_AS_WCS*sizeof(*str));
	verify(str);
	int str_len = 0;
	str_len += swprintf(str, UNE_SIZE_TOKEN_AS_WCS, UNE_COLOR_TOKEN_KIND);
	
	assert(UNE_TOKEN_KIND_IS_VALID(token.kind));
	str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L"%ls", une_token_kind_to_wcs(token.kind));
	str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, UNE_COLOR_RESET);
	
	/* Write token value if it exists. */
	switch (token.kind) {
		
		/* Numbers. */
		case UNE_TK_INT:
			str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE UNE_PRINTF_UNE_INT UNE_COLOR_RESET, token.value._int);
			break;
		case UNE_TK_FLT: {
			wchar_t *flt_as_wcs = une_flt_to_wcs(token.value._flt);
			str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE L"%ls" UNE_COLOR_RESET, flt_as_wcs);
			free(flt_as_wcs);
			break;
		}
		
		/* Strings. */
		case UNE_TK_STR:
			str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE L"\"%ls" UNE_COLOR_TOKEN_VALUE L"\"" UNE_COLOR_RESET, token.value._wcs);
			break;
		case UNE_TK_NAME:
			str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE L"%ls" UNE_COLOR_RESET, token.value._wcs);
			break;
		
		/* Native function. */
		case UNE_TK_NATIVE:
			str_len += swprintf(str+wcslen(str), UNE_SIZE_TOKEN_AS_WCS, L":" UNE_COLOR_TOKEN_VALUE L"%ls" UNE_COLOR_RESET, une_native_to_wcs((une_native)token.value._int));
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
	if (tokens == NULL) {
		wprintf(L"No tokens");
		return;
	}
	
	/* Print the text representation of each token. */
	size_t i = 0;
	wchar_t *token_as_wcs;
	while (true) {
		token_as_wcs = une_token_to_wcs(tokens[i]);
		wprintf(L"%ls ", token_as_wcs);
		free(token_as_wcs);
		if (tokens[i].kind == UNE_TK_EOF)
			break;
		i++;
	}
}
#endif /* UNE_DEBUG */
