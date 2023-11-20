/*
lexer.c - Une
Modified 2023-11-20
*/

/* Header-specific includes. */
#include "lexer.h"

/* Implementation-specific includes. */
#include <string.h>
#include "tools.h"
#include "builtin_functions.h"

/*
Public lexer interface.
*/

void une_lex(une_error *error, une_lexer_state *ls)
{
	assert(ls->text);
	
	while (true) {
		/* Check for error. */
		if (error->type != UNE_ET_none__) {
			for (size_t i=0; i<ls->tokens_count - 1 /* Don't free UNE_TT_none__. */ ; i++)
				une_token_free(ls->tokens[i]);
			free(ls->tokens);
			ls->tokens = NULL;
			break;
		}
		
		/* Begin string expression. */
		if (ls->begin_str_expression) {
			ls->begin_str_expression = false;
			ls->in_str_expression = true;
			une_lexer_commit(ls, (une_token){
				.type = UNE_TT_STR_EXPRESSION_BEGIN,
				.pos = (une_position){
					.start = ls->text_index-1,
					.end = ls->text_index,
					.line = ls->line
				},
				.value._vp = 0
			});
		}
		
		/* Expected end of file. */
		if (une_lexer_now(ls) == L'\0') {
			if (ls->tokens_count > 0 && ls->tokens[ls->tokens_count-1].type != UNE_TT_NEW)
				une_lexer_commit(ls, (une_token){
					.type = UNE_TT_NEW,
					.pos = (une_position){
						.start = ls->text_index,
						.end = ls->text_index+1,
						.line = ls->line
					},
					.value._vp = NULL
				});
			une_lexer_commit(ls, (une_token){
				.type = UNE_TT_EOF,
				.pos = (une_position){
					.start = ls->text_index,
					.end = ls->text_index+1,
					.line = ls->line
				},
				.value._vp = NULL
			});
			break;
		}
		
		/* Skip whitespace. */
		if (UNE_LEXER_WC_IS_SOFT_WHITESPACE(une_lexer_now(ls))) {
			while (UNE_LEXER_WC_IS_SOFT_WHITESPACE(une_lexer_now(ls)))
				une_lexer_advance(ls);
			continue;
		}
		
		/* Number. */
		if (UNE_LEXER_WC_IS_DIGIT(une_lexer_now(ls))) {
			une_lexer_commit(ls, une_lex_number(error, ls, false));
			continue;
		}
		
		/* String. */
		if (une_lexer_now(ls) == L'"') {
			une_lexer_commit(ls, une_lex_string(error, ls));
			continue;
		}
		if (ls->in_str_expression && une_lexer_now(ls) == L'}') {
			une_lexer_commit(ls, (une_token){
				.type = UNE_TT_STR_EXPRESSION_END,
				.pos = (une_position){
					.start = ls->text_index,
					.end = ls->text_index+1,
					.line = ls->line
				},
				.value._vp = 0
			});
			ls->in_str_expression = false;
			une_lexer_commit(ls, une_lex_string(error, ls));
			continue;
		}
		
		/* Operator. */
		une_token tk = une_lex_operator(error, ls);
		if (tk.type != UNE_TT_none__) {
			une_lexer_commit(ls, tk);
			continue;
		}

		/* Keyword or name. */
		if (UNE_LEXER_WC_CAN_BEGIN_NAME(une_lexer_now(ls))) {
			une_lexer_commit(ls, une_lex_keyword_or_name(error, ls));
			continue;
		}
		
		/* NEW. */
		if (UNE_LEXER_WC_IS_HARD_WHITESPACE(une_lexer_now(ls))) {
			size_t idx_left = ls->text_index;
			size_t lines = 0;
			while (UNE_LEXER_WC_IS_WHITESPACE(une_lexer_now(ls))) {
				if (une_lexer_now(ls) == '\n')
					lines++;
				une_lexer_advance(ls);
			}
			if (ls->tokens_count > 0 && ls->tokens[ls->tokens_count-1].type != UNE_TT_NEW)
				une_lexer_commit(ls, (une_token){
					.type = UNE_TT_NEW,
					.pos = (une_position){
						.start = idx_left,
						.end = ls->text_index,
						.line = ls->line
					},
					.value._vp = 0
				});
			ls->line += lines;
			continue;
		}
		
		/* Comment. */
		if (une_lexer_now(ls) == L'#') {
			do
				une_lexer_advance(ls);
			while (une_lexer_now(ls) != L'\0' && une_lexer_now(ls) != L'\n');
			continue;
		}
		
		/* Unexpected character. */
		*error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
			.start = ls->text_index,
			.end = ls->text_index+1,
			.line = ls->line
		}));
		une_lexer_commit(ls, une_token_create(UNE_TT_none__));
		continue;
	}
}

/*
Lexers.
*/

une_lexer__(une_lex_operator)
{
	for (une_token_type tt=UNE_R_BGN_OPERATOR_TOKENS; tt<=UNE_R_END_OPERATOR_TOKENS; tt++) {
		ptrdiff_t i=0;
		while (une_lexer_peek(ls, i) == une_token_table[tt-1][i]) {
			if (une_token_table[tt-1][++i] == L'\0') {
				ptrdiff_t starting_index = (ptrdiff_t)ls->text_index;
				for (; i>0; i--)
					une_lexer_advance(ls);
				return (une_token){
					.type = tt,
					.pos = (une_position){
						.start = (size_t)starting_index,
						.end = ls->text_index,
						.line = ls->line
					}
				};
			}
		}
	}
	return une_token_create(UNE_TT_none__);
}

une_lexer__(une_lex_number, bool allow_signed)
{
	size_t start_index = ls->text_index;
	
	int base = 0;
	if (!une_lex_number_base(error, ls, &base))
		return une_token_create(UNE_TT_none__);
	
	une_int integer = 0;
	if (!une_lex_number_integer(error, ls, base, &integer, allow_signed, true, 0))
		return une_token_create(UNE_TT_none__);
	
	une_flt floating = (une_flt)integer;
	bool is_floating = une_lexer_now(ls) == L'.' && une_lexer_peek(ls, 1) != L'.'; /* Not '..'. */
	if (is_floating && !une_lex_number_fractional_part(error, ls, base, &floating))
		return une_token_create(UNE_TT_none__);
	
	une_int exponent = 0;
	bool has_exponent = base == 10 && (une_lexer_now(ls) == L'e' || une_lexer_now(ls) == L'E');
	if (has_exponent) {
		if (!une_lex_number_exponent(error, ls, &exponent))
			return une_token_create(UNE_TT_none__);
		if (!is_floating) {
			is_floating = true;
			floating = (une_flt)integer;
		}
		floating *= powl(10.0L, (une_flt)exponent);
	}
	
	une_token number = {
		.type = is_floating ? UNE_TT_FLT : UNE_TT_INT,
		.pos = (une_position){
			.start = start_index,
			.end = ls->text_index,
			.line = ls->line
		}
	};
	if (is_floating)
		number.value._flt = floating;
	else
		number.value._int = integer;
	return number;
}

une_lexer__(une_lex_string)
{
	/* Setup. */
	size_t buffer_size = UNE_SIZE_STR_LEN;
	wchar_t *buffer = malloc(buffer_size*sizeof(*buffer));
	verify(buffer);
	size_t idx_start = ls->text_index;
	size_t line_start = ls->line; /* Strings can contain newlines. */
	bool escape = false;
	size_t buffer_index = 0;
	
	while (true) {
		/* Ensure sufficient space in string buffer. */
		if (buffer_index+1 /* NUL. */ >= buffer_size) {
			buffer_size *= 2;
			buffer = realloc(buffer, buffer_size*sizeof(*buffer));
			verify(buffer);
		}
		
		une_lexer_advance(ls);
		
		/* Correct lexer state line. */
		if (une_lexer_now(ls) == L'\n')
			ls->line++;
		
		/* Ignore carriage return. */
		if (une_lexer_now(ls) == L'\r')
			continue;
		
		/* Premature end of string. */
		if (une_lexer_now(ls) == L'\0') {
			*error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
				.start = ls->text_index,
				.end = ls->text_index+1,
				.line = ls->line
			}));
			free(buffer);
			return une_token_create(UNE_TT_none__);
		}
		
		/* Escaped characters. */
		if (escape) {
			escape = false;
			switch (une_lexer_now(ls)) {
				case L'\\':
				case L'"':
				case L'{':
				case L'}':
					buffer[buffer_index++] = une_lexer_now(ls);
					continue;
				case L'n':
					buffer[buffer_index++] = L'\n';
					continue;
				case L'r':
					buffer[buffer_index++] = L'\r';
					continue;
				case L't':
					buffer[buffer_index++] = L'\t';
					continue;
				case L'e':
					buffer[buffer_index++] = L'\033';
					continue;
				case L'a':
					buffer[buffer_index++] = L'\a';
					continue;
				case L'\n':
					continue;
				case L'o': {
					une_lexer_advance(ls);
					une_int integer = 0;
					if (!une_lex_number_integer(error, ls, 8, &integer, false, false, 2))
						break;
					ls->text_index--; /* Backtrack because we advance at the beginning of each loop. */
					buffer[buffer_index++] = (wchar_t)integer;
					continue;
				}
				case L'x': {
					une_lexer_advance(ls);
					une_int integer = 0;
					if (!une_lex_number_integer(error, ls, 16, &integer, false, false, 2))
						break;
					ls->text_index--; /* Backtrack because we advance at the beginning of each loop. */
					buffer[buffer_index++] = (wchar_t)integer;
					continue;
				}
			}
			*error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
				.start = ls->text_index,
				.end = ls->text_index+1,
				.line = ls->line
			}));
			free(buffer);
			return une_token_create(UNE_TT_none__);
		}
		
		/* Schedule escaped character. */
		if (une_lexer_now(ls) == L'\\') {
			escape = true;
			continue;
		}
		
		/* End of string. */
		if (une_lexer_now(ls) == L'"') {
			une_lexer_advance(ls);
			break;
		}
		
		/* Beginning of string expression (end of this string). */
		if (une_lexer_now(ls) == L'{') {
			if (ls->in_str_expression) {
				*error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
					.start = ls->text_index,
					.end = ls->text_index+1,
					.line = ls->line
				}));
				free(buffer);
				return une_token_create(UNE_TT_none__);
			}
			une_lexer_advance(ls);
			ls->begin_str_expression = true;
			break;
		}
		
		/* Add character to string buffer. */
		buffer[buffer_index++] = (wchar_t)une_lexer_now(ls);
	}
	
	buffer[buffer_index] = L'\0';
	return (une_token){
		.type = UNE_TT_STR,
		.pos = (une_position){
			.start = idx_start,
			.end = ls->text_index,
			.line = line_start
		},
		/* DOC: Memory Management: This is where strings referenced during tokenization and parsing are constructed. */
		.value._wcs = buffer,
	};
}

une_lexer__(une_lex_keyword_or_name)
{
	size_t buffer_size = UNE_SIZE_NAME_LEN;
	wchar_t *buffer = malloc(buffer_size*sizeof(*buffer));
	verify(buffer);
	size_t idx_start = ls->text_index;
	size_t buffer_index = 0;
	
	/* Read keyword or name. */
	while (UNE_LEXER_WC_CAN_BE_IN_NAME(une_lexer_now(ls))) {
		if (buffer_index >= buffer_size-1) {
			buffer_size *= 2;
			buffer = realloc(buffer, buffer_size*sizeof(*buffer));
			verify(buffer);
		}
		buffer[buffer_index++] = (wchar_t)une_lexer_now(ls);
		une_lexer_advance(ls);
	}
	buffer[buffer_index] = L'\0';
	
	/* Determine token type. */
	une_token tk;
	if (!wcscmp(buffer, une_token_table[UNE_TT_FUNCTION-1])) {
		tk.type = UNE_TT_FUNCTION;
		tk.value._vp = (void*)ls->name;
		goto keyword_or_name_defined;
	}
	for (une_token_type tt=UNE_R_BGN_KEYWORD_TOKENS; tt<=UNE_R_END_KEYWORD_TOKENS; tt++)
		if (!wcscmp(buffer, une_token_table[tt-1])) {
			tk.type = tt;
			goto keyword_or_name_defined;
		}
	une_builtin_function builtin = une_builtin_wcs_to_function(buffer);
	if (builtin != UNE_BUILTIN_none__) {
		tk.type = UNE_TT_BUILTIN;
		tk.value._int = (une_int)builtin;
		goto keyword_or_name_defined;
	}
	tk.type = UNE_TT_NAME;
	tk.value._wcs = buffer;
	
	/* Finalize token. */
	keyword_or_name_defined:
	if (tk.type != UNE_TT_NAME)
		free(buffer);
	tk.pos = (une_position){
		.start = idx_start,
		.end = ls->text_index,
		.line = ls->line
	};
	
	return tk;
}

bool une_lex_number_base(une_error *error, une_lexer_state *ls, int *base)
{
	*base = 10;
	if (une_lexer_now(ls) == L'0' && UNE_LEXER_WC_CAN_BEGIN_NAME(une_lexer_peek(ls, 1))) {
		switch (une_lexer_advance(ls) /* '0'. */ ) {
			case L'b': *base = 2; break;
			case L'o': *base = 8; break;
			case L'x': *base = 16; break;
			default:
				*error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
					.start = ls->text_index,
					.end = ls->text_index+1,
					.line = ls->line
				}));
				return false;
		}
		une_lexer_advance(ls); /* Base name. */
	}
	return true;
}

bool une_lex_number_integer(une_error *error, une_lexer_state *ls, int base, une_int *integer, bool allow_signed, bool allow_e, size_t limit_length)
{
	int sign = 1;
	if (allow_signed && une_lexer_now(ls) == L'-') {
		une_lexer_advance(ls);
		sign = -1;
	}
	
	size_t index_start = ls->text_index;
	while (une_lexer_now(ls) == L'0')
		une_lexer_advance(ls);
	*integer = 0;
	int digit_in_decimal;
	
	while (true) {
		if (
			!une_lexer_digit_to_decimal(une_lexer_now(ls), &digit_in_decimal) ||
			(allow_e && base == 10 && (une_lexer_now(ls) == L'e' || une_lexer_now(ls) == 'E')) ||
			(limit_length && ls->text_index == index_start + limit_length)
		)
			break;
		if (digit_in_decimal >= base) {
			*error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
				.start = ls->text_index,
				.end = ls->text_index+1,
				.line = ls->line
			}));
			return false;
		}
		*integer *= base;
		*integer += digit_in_decimal;
		une_lexer_advance(ls);
	}
	
	if (ls->text_index == index_start) {
		*error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
			.start = ls->text_index,
			.end = ls->text_index+1,
			.line = ls->line
		}));
		return false;
	}
	*integer *= sign;
	return true;
}

bool une_lex_number_fractional_part(une_error *error, une_lexer_state *ls, int base, une_flt *floating)
{
	assert(une_lexer_now(ls) == L'.' && une_lexer_peek(ls, 1) != L'.');
	une_lexer_advance(ls); /* Radix point. */
	
	une_flt floating_divisor = 1;
	int digit_in_decimal;
	
	while (true) {
		if (
			!une_lexer_digit_to_decimal(une_lexer_now(ls), &digit_in_decimal) ||
			(base == 10 && (une_lexer_now(ls) == L'e' || une_lexer_now(ls) == 'E'))
		)
			break;
		if (digit_in_decimal >= base) {
			*error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
				.start = ls->text_index,
				.end = ls->text_index+1,
				.line = ls->line
			}));
			return false;
		}
		floating_divisor *= base;
		*floating += digit_in_decimal / floating_divisor;
		une_lexer_advance(ls);
	}
	
	/* Missing fraction. */
	if (floating_divisor <= 1) {
		*error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
			.start = ls->text_index,
			.end = ls->text_index+1,
			.line = ls->line
		}));
		return false;
	}
	
	return true;
}

bool une_lex_number_exponent(une_error *error, une_lexer_state *ls, une_int *exponent)
{
	assert(une_lexer_now(ls) == L'e' || une_lexer_now(ls) == L'E');
	une_lexer_advance(ls); /* 'e' or 'E'. */
	
	return une_lex_number_integer(error, ls, 10, exponent, true, false, 0);
}

/*
Helpers.
*/

wchar_t une_lexer_now(une_lexer_state *ls)
{
	assert(ls);
	assert(ls->text);
	assert(ls->text_index <= ls->text_length);
	return ls->text[ls->text_index];
}

wchar_t une_lexer_advance(une_lexer_state *ls)
{
	assert(ls);
	assert(ls->text);
	assert(ls->text_index <= ls->text_length);
	if (ls->text_index + 1 <= ls->text_length)
		ls->text_index++;
	return ls->text[ls->text_index];
}

wchar_t une_lexer_peek(une_lexer_state *ls, une_int offset)
{
	assert(ls);
	assert(ls->text);
	assert(ls->text_index <= ls->text_length);
	return ls->text[une_clamp((une_int)ls->text_index+offset, 0, (une_int)ls->text_length)];
}

void une_lexer_commit(une_lexer_state *ls, une_token token)
{
	assert(ls);
	assert(ls->tokens);
	
	if (ls->tokens_count >= ls->tokens_size) {
		ls->tokens_size *= 2;
		ls->tokens = realloc(ls->tokens, ls->tokens_size*sizeof(*ls->tokens));
		verify(ls->tokens);
	}
	
	ls->tokens[ls->tokens_count++] = token;
}

bool une_lexer_digit_to_decimal(wchar_t digit, int *digit_in_decimal)
{
	if (UNE_LEXER_WC_IS_DIGIT(digit))
		*digit_in_decimal = digit - L'0';
	else if (UNE_LEXER_WC_IS_LOWERCASE_LETTER(digit))
		*digit_in_decimal = 10 + digit - L'a';
	else if (UNE_LEXER_WC_IS_UPPERCASE_LETTER(digit))
		*digit_in_decimal = 10 + digit - L'A';
	else
		return false;
	return true;
}
