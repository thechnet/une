/*
builtin.h - Une
Modified 2021-08-11
*/

#ifndef UNE_BUILTIN_H
#define UNE_BUILTIN_H

/* Header-specific includes. */
#include "primitive.h"
#include "types/result.h"
#include "types/error.h"
#include "types/interpreter_state.h"

#define __une_builtin_fn_sign(__id) une_result (__id)(une_error *error, une_interpreter_state *is, une_node *call_node, une_result *args)

typedef const int une_builtin_param;
typedef __une_builtin_fn_sign(*une_builtin_fnptr);

/*
*** Interface.
*/

/*
Every built-in function.
*/
#define UNE_ENUMERATE_BUILTIN_FUNCTIONS(enumerator) \
  enumerator(put) \
  enumerator(print) \
  enumerator(int) \
  enumerator(flt) \
  enumerator(str) \
  enumerator(len) \
  enumerator(sleep) \
  enumerator(chr) \
  enumerator(ord) \
  enumerator(read) \
  enumerator(write) \
  enumerator(input) \
  enumerator(script) \
  enumerator(exist) \
  enumerator(split)

/*
The index of a built-in function.
*/
typedef enum _une_builtin_function {
  __UNE_BUILTIN_none__,
  #define __BUILTIN_FUNCTION(__id) UNE_BUILTIN_##__id,
  UNE_ENUMERATE_BUILTIN_FUNCTIONS(__BUILTIN_FUNCTION)
  #undef __BUILTIN_FUNCTION
  __UNE_BUILTIN_max__,
} une_builtin_function;

/*
Built-in function template.
*/
#define __une_builtin_fn(__id) __une_builtin_fn_sign(une_builtin_fn_##__id)

/*
Get the position of an argument.
*/
#define UNE_BUILTIN_POS_OF_ARG(index) \
  (((une_node**)call_node->content.branch.b->content.value._vpp)[index+1]->pos)

/*
Ensure an argument has the correct type.
*/
#define UNE_BUILTIN_VERIFY_ARG_TYPE(index, expected_type) \
  if (args[index].type != expected_type) {\
    *error = UNE_ERROR_SET(UNE_ET_TYPE, UNE_BUILTIN_POS_OF_ARG(index));\
    return une_result_create(UNE_RT_ERROR);\
  }

/*
Verify that a une_builtin_function is valid.
*/
#define UNE_BUILTIN_FUNCTION_IS_VALID(function) \
  (function > __UNE_BUILTIN_none__ && function < __UNE_BUILTIN_max__)

const size_t une_builtin_params_count(une_builtin_function function);
const une_builtin_fnptr une_builtin_function_to_fnptr(une_builtin_function function);
une_builtin_function une_builtin_wcs_to_function(wchar_t *wcs);

#ifdef UNE_DEBUG
const wchar_t *une_builtin_function_to_wcs(une_builtin_function function);
#endif /* UNE_DEBUG */

#define __BUILTIN_DECLARATION(__id) __une_builtin_fn(__id);
UNE_ENUMERATE_BUILTIN_FUNCTIONS(__BUILTIN_DECLARATION)
#undef __BUILTIN_DECLARATION

#endif /* UNE_BUILTIN_H */
