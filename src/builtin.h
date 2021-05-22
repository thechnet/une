/*
builtin.h - Une
Modified 2021-05-22
*/

#ifndef UNE_BUILTIN_H
#define UNE_BUILTIN_H

#include "primitive.h"
#include "types/result.h"
#include "types/error.h"
#include "types/instance.h"

typedef enum _une_builtin_type {
  UNE_BIF_NONE,
  UNE_BIF_PRINT,
  UNE_BIF_TO_INT,
  UNE_BIF_TO_FLT,
  UNE_BIF_TO_STR,
  UNE_BIF_GET_LEN,
  __UNE_BIF_max__
} une_builtin_type;

const une_builtin_type une_builtin_wcs_to_type(wchar_t *name);
const une_int une_builtin_get_num_of_params(une_builtin_type type);

une_result une_builtin_print  (une_instance *inst, une_position pos, une_result result);
une_result une_builtin_to_int (une_instance *inst, une_position pos, une_result result);
une_result une_builtin_to_flt (une_instance *inst, une_position pos, une_result result);
une_result une_builtin_to_str (une_instance *inst, une_position pos, une_result result);
une_result une_builtin_get_len(une_instance *inst, une_position pos, une_result result);

#endif /* UNE_BUILTIN_H */