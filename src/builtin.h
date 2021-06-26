/*
builtin.h - Une
Modified 2021-06-26
*/

#ifndef UNE_BUILTIN_H
#define UNE_BUILTIN_H

/* Header-specific includes. */
#include "primitive.h"
#include "types/result.h"
#include "types/error.h"
#include "types/interpreter_state.h"

/*
The type of built-in function.
*/
typedef enum _une_builtin_type {
  __UNE_BIF_none__,
  UNE_BIF_PRINT,
  UNE_BIF_TO_INT,
  UNE_BIF_TO_FLT,
  UNE_BIF_TO_STR,
  UNE_BIF_GET_LEN,
  UNE_BIF_SLEEP,
  UNE_BIF_CHR,
  UNE_BIF_ORD,
  UNE_BIF_READ,
  UNE_BIF_WRITE,
  UNE_BIF_INPUT,
  __UNE_BIF_max__,
} une_builtin_type;

/*
*** Interface.
*/

/*
Built-in function template.
*/
#define __une_builtin_fn(__id, ...) une_result (__id)(une_error *error, une_interpreter_state *is, une_position pos, ##__VA_ARGS__)

const une_builtin_type une_builtin_wcs_to_type(wchar_t *name);
const une_int une_builtin_get_num_of_params(une_builtin_type type);

__une_builtin_fn(une_builtin_print,   une_result result);
__une_builtin_fn(une_builtin_to_int,  une_result result);
__une_builtin_fn(une_builtin_to_flt,  une_result result);
__une_builtin_fn(une_builtin_to_str,  une_result result);
__une_builtin_fn(une_builtin_get_len, une_result result);
__une_builtin_fn(une_builtin_sleep,   une_result result);
__une_builtin_fn(une_builtin_chr,     une_result result);
__une_builtin_fn(une_builtin_ord,     une_result result);
__une_builtin_fn(une_builtin_read,    une_result result);
__une_builtin_fn(une_builtin_write,   une_result file, une_position pos2, une_result text);
__une_builtin_fn(une_builtin_input,   une_result result);

#endif /* UNE_BUILTIN_H */
