/*
builtin.c - Une
Modified 2021-08-14
*/

/* Header-specific includes. */
#include "builtin.h"

/* Implementation-specific includes. */
#include "../types/symbols.h"
#include "../builtin_functions.h"

/*
Print a text representation to file.
*/
void une_datatype_builtin_represent(FILE *file, une_result result)
{
  assert(result.type == UNE_RT_BUILTIN);
  fwprintf(file, L"BUILTIN");
}

/*
Check for truth.
*/
une_int une_datatype_builtin_is_true(une_result result)
{
  assert(result.type == UNE_RT_BUILTIN);
  return 1;
}

/*
Call result.
*/
une_result une_datatype_builtin_call(une_error *error, une_interpreter_state *is, une_node *call, une_result function, une_result args)
{
  /* Get built-in function. */
  une_builtin_function builtin = (une_builtin_function)function.value._int;
  
  /* Ensure number of arguments matches number of required parameters. */
  UNE_UNPACK_RESULT_LIST(args, args_p, args_count);
  if (une_builtin_params_count(builtin) != args_count) {
    *error = UNE_ERROR_SET(UNE_ET_CALLABLE_ARG_COUNT, call->pos);
  }
  
  return (une_builtin_function_to_fnptr(builtin))(error, is, call, args_p+1);
}
