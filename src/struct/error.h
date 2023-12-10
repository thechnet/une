/*
error.h - Une
Modified 2023-12-10
*/

#ifndef UNE_ERROR_H
#define UNE_ERROR_H

/* Header-specific includes. */
#include "../common.h"
#include "lexer_state.h"
#include "interpreter_state.h"

/*
Kind of une_error.
*/
typedef enum une_error_kind_ {
	UNE_EK_none__,
	UNE_EK_SYNTAX,
	UNE_EK_BREAK_OUTSIDE_LOOP,
	UNE_EK_CONTINUE_OUTSIDE_LOOP,
	UNE_EK_SYMBOL_NOT_DEFINED,
	UNE_EK_INDEX,
	UNE_EK_ZERO_DIVISION,
	UNE_EK_UNREAL_NUMBER,
	UNE_EK_CALLABLE_ARG_COUNT,
	UNE_EK_FILE,
	UNE_EK_ENCODING,
	UNE_EK_TYPE,
	UNE_EK_ASSERTION_NOT_MET,
	UNE_EK_MISPLACED_ANY_OR_ALL,
	UNE_EK_SYSTEM,
	UNE_EK_max__,
} une_error_kind;

/*
Holds error information.
*/
typedef struct une_error_ {
	une_error_kind kind;
	une_position pos;
	char *meta_file;
	size_t meta_line;
} une_error;

/*
Holds a trace.
*/
typedef struct une_trace_ {
	char *file;
	une_position point;
	wchar_t *function_label;
	char *function_file;
	une_position function_point;
} une_trace;

/*
*** Interface.
*/

/*
Condition to check whether une_error_kind is valid.
*/
#define UNE_ERROR_KIND_IS_VALID(kind)\
	(kind > UNE_EK_none__ && kind < UNE_EK_max__)

/*
Populate a une_error.
*/
#define UNE_ERROR_SET(kind__, pos__)\
	(une_error){\
		.kind = kind__,\
		.pos = pos__,\
		.meta_line = __LINE__,\
		.meta_file = __FILE__,\
	}

une_error une_error_create(void);
const wchar_t *une_error_kind_to_wcs(une_error_kind kind);

#endif /* !UNE_ERROR_H */
