/*
natives.h - Une
Modified 2023-12-10
*/

#ifndef UNE_NATIVE_H
#define UNE_NATIVE_H

/* Header-specific includes. */
#include "common.h"
#include "struct/result.h"
#include "struct/error.h"
#include "struct/interpreter_state.h"
#include "struct/engine.h"

#define une_native_fn_sign__(name__) une_result (name__)(une_node *call_node, une_result *args)

typedef const int une_native_param;
typedef une_native_fn_sign__(*une_native_fnptr);

/*
*** Interface.
*/

/*
Every native function.
*/
#define UNE_ENUMERATE_NATIVE_FUNCTIONS(enumerator) \
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
The index of a native function.
*/
typedef enum une_native_ {
	UNE_NATIVE_none__,
	#define NATIVE_FUNCTION__(name__) UNE_NATIVE_##name__,
	UNE_ENUMERATE_NATIVE_FUNCTIONS(NATIVE_FUNCTION__)
	#undef NATIVE_FUNCTION__
	UNE_NATIVE_max__,
} une_native;

/*
Native function template.
*/
#define une_native_fn__(name__) une_native_fn_sign__(une_native_fn_##name__)

/*
Get the position of an argument.
*/
#define UNE_NATIVE_POS_OF_ARG(index) \
	(((une_node**)call_node->content.branch.b->content.value._vpp)[index+1]->pos)

/*
Ensure an argument has the correct kind.
*/
#define UNE_NATIVE_VERIFY_ARG_KIND(index, expected_kind) \
	do if (args[index].kind != expected_kind) {\
		felix->error = UNE_ERROR_SET(UNE_EK_TYPE, UNE_NATIVE_POS_OF_ARG(index));\
		return une_result_create(UNE_RK_ERROR);\
	} while(false)

/*
Verify that a une_native is valid.
*/
#define UNE_NATIVE_IS_VALID(function) \
	(function > UNE_NATIVE_none__ && function < UNE_NATIVE_max__)

size_t une_native_params_count(une_native function);
une_native_fnptr une_native_to_fnptr(une_native function);
une_native une_native_wcs_to_function(wchar_t *wcs);

#ifdef UNE_DEBUG
const wchar_t *une_native_to_wcs(une_native function);
#endif /* UNE_DEBUG */

#define NATIVE_DECLARATION__(name__) une_native_fn__(name__);
UNE_ENUMERATE_NATIVE_FUNCTIONS(NATIVE_DECLARATION__)
#undef NATIVE_DECLARATION__

#endif /* UNE_NATIVE_H */
