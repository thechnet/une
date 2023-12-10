/*
error.c - Une
Modified 2023-12-10
*/

/* Header-specific includes. */
#include "error.h"

/* Implementation-specific includes. */
#include "../lexer.h"

/*
Error message table.
*/
const wchar_t *une_error_message_table[] = {
	L"No error defined! (Internal Error)",
	L"Syntax.",
	L"Break outside loop.",
	L"Continue outside loop.",
	L"Symbol not defined.",
	L"Invalid index or slice.",
	L"Zero division.",
	L"Unreal number.",
	L"Wrong number of arguments.",
	L"File unsuitable or non-existent.",
	L"Encoding, conversion, or pattern error.",
	L"Type.",
	L"Assertion not met.",
	L"Misplaced 'any' or 'all'.",
	L"System error.",
	L"Unknown error! (Internal Error)",
};

/*
Initialize a une_error struct.
*/
une_error une_error_create(void)
{
	return (une_error){
		.pos = (une_position){
			.start = 0,
			.end = 0
		},
		.kind = UNE_EK_none__,
		.meta_line = 0,
		.meta_file = NULL
	};
}

/*
Get error message for error kind.
*/
const wchar_t *une_error_kind_to_wcs(une_error_kind kind)
{
	/* Ensure error kind is within bounds. */
	if (!UNE_ERROR_KIND_IS_VALID(kind))
		kind = UNE_EK_max__;

	return une_error_message_table[kind];
}
