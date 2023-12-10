/*
native.c - Une
Modified 2023-12-10
*/

/* Header-specific includes. */
#include "native.h"

/* Implementation-specific includes. */
#include "../struct/association.h"
#include "../natives.h"

/*
Print a text representation to file.
*/
void une_type_native_represent(FILE *file, une_result result)
{
	assert(result.kind == UNE_RK_NATIVE);
	fwprintf(file, L"<native>");
}

/*
Check for truth.
*/
une_int une_type_native_is_true(une_result result)
{
	assert(result.kind == UNE_RK_NATIVE);
	return 1;
}

/*
Check if subject is equal to comparison.
*/
une_int une_type_native_is_equal(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_NATIVE);
	if (comparison.kind != UNE_RK_NATIVE)
		return 0;
	return subject.value._int == comparison.value._int;
}

/*
Call result.
*/
une_result une_type_native_call(une_node *call, une_result function, une_result args, wchar_t *label)
{
	/* Get native function. */
	une_native native = (une_native)function.value._int;
	assert(UNE_NATIVE_IS_VALID(native));
	
	/* Ensure number of arguments matches number of required parameters. */
	UNE_UNPACK_RESULT_LIST(args, args_p, args_count);
	if (une_native_params_count(native) != args_count) {
		felix->error = UNE_ERROR_SET(UNE_EK_CALLABLE_ARG_COUNT, call->pos);
		return une_result_create(UNE_RK_ERROR);
	}
	
	return (une_native_to_fnptr(native))(call, args_p+1);
}
