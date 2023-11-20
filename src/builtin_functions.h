/*
builtin.h - Une
Modified 2023-11-20
*/

#ifndef UNE_BUILTIN_H
#define UNE_BUILTIN_H

/* Header-specific includes. */
#include "primitive.h"
#include "types/result.h"
#include "types/error.h"
#include "types/interpreter_state.h"
#include "types/engine.h"

#define une_builtin_fn_sign__(name__) une_result (name__)(une_node *call_node, une_result *args)

typedef const int une_builtin_param;
typedef une_builtin_fn_sign__(*une_builtin_fnptr);

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
	enumerator(append) \
	enumerator(input) \
	enumerator(script) \
	enumerator(exist) \
	enumerator(split) \
	enumerator(eval) \
	enumerator(replace) \
	enumerator(join) \
	enumerator(sort) \
	enumerator(getwd) \
	enumerator(setwd) \
	enumerator(playwav)

/*
The index of a built-in function.
*/
typedef enum une_builtin_function_ {
	UNE_BUILTIN_none__,
	#define BUILTIN_FUNCTION__(name__) UNE_BUILTIN_##name__,
	UNE_ENUMERATE_BUILTIN_FUNCTIONS(BUILTIN_FUNCTION__)
	#undef BUILTIN_FUNCTION__
	UNE_BUILTIN_max__,
} une_builtin_function;

/*
Built-in function template.
*/
#define une_builtin_fn__(name__) une_builtin_fn_sign__(une_builtin_fn_##name__)

/*
Get the position of an argument.
*/
#define UNE_BUILTIN_POS_OF_ARG(index) \
	(((une_node**)call_node->content.branch.b->content.value._vpp)[index+1]->pos)

/*
Ensure an argument has the correct type.
*/
#define UNE_BUILTIN_VERIFY_ARG_TYPE(index, expected_type) \
	do if (args[index].type != expected_type) {\
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, UNE_BUILTIN_POS_OF_ARG(index));\
		return une_result_create(UNE_RT_ERROR);\
	} while(false)

/*
Verify that a une_builtin_function is valid.
*/
#define UNE_BUILTIN_FUNCTION_IS_VALID(function) \
	(function > UNE_BUILTIN_none__ && function < UNE_BUILTIN_max__)

size_t une_builtin_params_count(une_builtin_function function);
une_builtin_fnptr une_builtin_function_to_fnptr(une_builtin_function function);
une_builtin_function une_builtin_wcs_to_function(wchar_t *wcs);

#ifdef UNE_DEBUG
const wchar_t *une_builtin_function_to_wcs(une_builtin_function function);
#endif /* UNE_DEBUG */

#define BUILTIN_DECLARATION__(name__) une_builtin_fn__(name__);
UNE_ENUMERATE_BUILTIN_FUNCTIONS(BUILTIN_DECLARATION__)
#undef BUILTIN_DECLARATION__

#endif /* UNE_BUILTIN_H */
