/*
error.h - Une
Modified 2023-06-23
*/

#ifndef UNE_ERROR_H
#define UNE_ERROR_H

/* Header-specific includes. */
#include "../primitive.h"
#include "lexer_state.h"
#include "interpreter_state.h"

/*
Type of une_error.
*/
typedef enum une_error_type_ {
  UNE_ET_none__,
  UNE_ET_SYNTAX,
  UNE_ET_BREAK_OUTSIDE_LOOP,
  UNE_ET_CONTINUE_OUTSIDE_LOOP,
  UNE_ET_SYMBOL_NOT_DEFINED,
  UNE_ET_INDEX,
  UNE_ET_ZERO_DIVISION,
  UNE_ET_UNREAL_NUMBER,
  UNE_ET_CALLABLE_ARG_COUNT,
  UNE_ET_FILE_NOT_FOUND,
  UNE_ET_ENCODING,
  UNE_ET_TYPE,
  UNE_ET_ASSERTION_NOT_MET,
  UNE_ET_MISPLACED_ANY_OR_ALL,
  UNE_ET_max__,
} une_error_type;

/*
Holds error information.
*/
typedef struct une_error_ {
  une_error_type type;
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
Condition to check whether une_error_type is valid.
*/
#define UNE_ERROR_TYPE_IS_VALID(type)\
  (type > UNE_ET_none__ && type < UNE_ET_max__)

/*
Populate a une_error.
*/
#define UNE_ERROR_SET(type__, pos__)\
  (une_error){\
    .type = type__,\
    .pos = pos__,\
    .meta_line = __LINE__,\
    .meta_file = __FILE__,\
  }

une_error une_error_create(void);

size_t une_error_trace(une_error *error, une_lexer_state *ls, une_trace **out);
void une_error_trace_print(une_trace trace);
void une_error_display(une_error *error, une_lexer_state *ls);

#endif /* !UNE_ERROR_H */
