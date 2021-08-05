/*
builtin.h - Une
Modified 2021-08-05
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
Built-in function template.
*/
#define __une_builtin_fn(__id) __une_builtin_fn_sign(une_builtin_fn_##__id)

/*
Ensure the number of arguments received are correct.
*/
#define UNE_BUILTIN_VERIFY_ARG_COUNT(expected_argc) \
  if (((une_node**)call_node->content.branch.b->content.value._vpp)[0]->content.value._int != expected_argc) {\
    *error = UNE_ERROR_SET(UNE_ET_FUNCTION_ARG_COUNT, call_node->pos);\
    return une_result_create(UNE_RT_ERROR);\
  }

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

const size_t une_builtin_params_count(une_builtin_fnptr fn);
une_builtin_fnptr une_builtin_wcs_to_fnptr(wchar_t *wcs);

#define __BUILTIN_DECLARATION(__id) __une_builtin_fn(__id);
UNE_ENUMERATE_BUILTIN_FUNCTIONS(__BUILTIN_DECLARATION)
#undef __BUILTIN_DECLARATION

#endif /* UNE_BUILTIN_H */
