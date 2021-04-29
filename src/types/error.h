/*
error.h - Une
Updated 2021-04-29
*/

#ifndef UNE_ERROR_H
#define UNE_ERROR_H

#include "../primitive.h"
#include "token.h"
#include "result.h"

#pragma region une_error_type
typedef enum _une_error_type {
  UNE_ET_NO_ERROR, // Debugging
  UNE_ET_EXPECTED_TOKEN,
  UNE_ET_UNEXPECTED_TOKEN,
  UNE_ET_UNEXPECTED_CHARACTER,
  UNE_ET_INCOMPLETE_FLOAT,
  UNE_ET_ADD,
  UNE_ET_SUB,
  UNE_ET_MUL,
  UNE_ET_DIV,
  UNE_ET_FDIV,
  UNE_ET_MOD,
  UNE_ET_NEG,
  UNE_ET_POW,
  UNE_ET_ZERO_DIVISION,
  UNE_ET_COMPARISON,
  UNE_ET_NOT_INDEXABLE,
  UNE_ET_NOT_INDEX_TYPE,
  UNE_ET_INDEX_OUT_OF_RANGE,
  UNE_ET_SET,
  UNE_ET_SET_NOT_INDEXABLE,
  UNE_ET_GET,
  UNE_ET_FOR,
  UNE_ET_BREAK_OUTSIDE_LOOP,
  UNE_ET_CONTINUE_OUTSIDE_LOOP,
  UNE_ET_UNTERMINATED_STRING,
  UNE_ET_CANT_ESCAPE_CHAR,
  UNE_ET_UNREAL_NUMBER,
  UNE_ET_SET_NO_ID,
  UNE_ET_DEF,
  UNE_ET_CALL,
  UNE_ET_CALL_ARGS,
  UNE_ET_EXPECTED_RESULT_TYPE,
  UNE_ET_PRINT_VOID,
  UNE_ET_CONVERSION,
  UNE_ET_GETLEN,
  __UNE_ET_max__
} une_error_type;
#pragma endregion une_error_type

#pragma region une_error
typedef struct _une_error {
  une_error_type type;
  une_position pos;
  une_value values[2]; /* If you change this number, don't forget to change it in
                          une_error_free and UNE_ERROR_SET */
  char *__file__;  // Debugging
  size_t __line__; // ''
} une_error;
#pragma endregion une_error

wchar_t *une_error_value_to_wcs(une_error_type type, une_value *values);
void une_error_display(une_error error, wchar_t *text, wchar_t *name);
void une_error_free(une_error error);
une_error une_error_copy(une_error src);

#define UNE_ERROR_SETX(__type, __pos, __v0, __v1)\
  (une_error){\
    .type = __type,\
    .pos = __pos,\
    .__line__ = __LINE__,\
    .__file__ = __FILE__,\
    .values[0].__v0,\
    .values[1].__v1,\
  }
#define UNE_ERROR_SET(__type, __pos)\
  (une_error){\
    .type = __type,\
    .pos = __pos,\
    .__line__ = __LINE__,\
    .__file__ = __FILE__,\
  }

#endif /* !UNE_ERROR_H */
