/*
error.h - Une
Modified 2021-09-15
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
  UNE_ET_SYNTAX,
  UNE_ET_BREAK_OUTSIDE_LOOP,
  UNE_ET_CONTINUE_OUTSIDE_LOOP,
  UNE_ET_SYMBOL_NOT_DEFINED,
  UNE_ET_INDEX_OUT_OF_RANGE,
  UNE_ET_ZERO_DIVISION,
  UNE_ET_UNREAL_NUMBER,
  UNE_ET_CALLABLE_ARG_COUNT,
  UNE_ET_FILE_NOT_FOUND,
  UNE_ET_ENCODING,
  UNE_ET_TYPE,
  __UNE_ET_max__,
} une_error_type;

/*
Holds error information.
*/
typedef struct _une_error {
  une_error_type type;
  une_position pos;
  char *meta_file;
  size_t meta_line;
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
    .meta_line = __LINE__,\
    .meta_file = __FILE__,\
  }

une_error une_error_create(void);

void une_error_display(une_error *error, une_lexer_state *ls, une_interpreter_state *is);

#endif /* !UNE_ERROR_H */
