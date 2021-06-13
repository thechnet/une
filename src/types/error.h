/*
error.h - Une
Updated 2021-06-09
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
typedef enum _une_error_type {
  __UNE_ET_none__,
  UNE_ET_UNEXPECTED_CHARACTER,
  UNE_ET_SYNTAX,
  UNE_ET_BREAK_OUTSIDE_LOOP,
  UNE_ET_CONTINUE_OUTSIDE_LOOP,
  UNE_ET_ASSIGN_TO_LITERAL,
  UNE_ET_VARIABLE_NOT_DEFINED,
  UNE_ET_OPERATION_NOT_SUPPORTED,
  UNE_ET_COMPARISON_NOT_SUPPORTED,
  UNE_ET_INDEX_CANNOT_GET,
  UNE_ET_INDEX_CANNOT_SET,
  UNE_ET_INDEX_TYPE_NOT_SUPPORTED,
  UNE_ET_INDEX_OUT_OF_RANGE,
  UNE_ET_ZERO_DIVISION,
  UNE_ET_UNREAL_NUMBER,
  UNE_ET_FUNCTION_ALREADY_DEFINED,
  UNE_ET_FUNCTION_NOT_DEFINED,
  UNE_ET_FUNCTION_ARGC,
  UNE_ET_FUNCTION_UNEXPECTED_ARG_TYPE,
  __UNE_ET_max__,
} une_error_type;

/*
Holds error information.
*/
typedef struct _une_error {
  une_error_type type;
  une_position pos;
  char *file;
  size_t line;
} une_error;

/*
*** Interface.
*/

/*
Condition to check whether une_error_type is valid.
*/
#define UNE_ERROR_TYPE_IS_VALID(type)\
  (type > __UNE_ET_none__ && type < __UNE_ET_max__)

/*
Populate a une_error.
*/
#define UNE_ERROR_SET(__type, __pos)\
  (une_error){\
    .type = __type,\
    .pos = __pos,\
    .line = __LINE__,\
    .file = __FILE__,\
  }

une_error une_error_create(void);

__une_static const wchar_t *une_error_type_to_wcs(une_error_type type);

void une_error_display(une_error *error, une_lexer_state *ls, une_interpreter_state *is);

#endif /* !UNE_ERROR_H */
