/*
builtin.c - Une
Modified 2023-12-01
*/

/* Header-specific includes. */
#include "builtin.h"

/* Implementation-specific includes. */
#include "../types/association.h"
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
Check if subject is equal to comparison.
*/
une_int une_datatype_builtin_is_equal(une_result subject, une_result comparison)
{
	assert(subject.type == UNE_RT_BUILTIN);
	if (comparison.type != UNE_RT_BUILTIN)
		return 0;
	return subject.value._int == comparison.value._int;
}

/*
Call result.
*/
une_result une_datatype_builtin_call(une_node *call, une_result function, une_result args, wchar_t *label)
{
	/* Get built-in function. */
	une_builtin_function builtin = (une_builtin_function)function.value._int;
	assert(UNE_BUILTIN_FUNCTION_IS_VALID(builtin));
	
	/* Ensure number of arguments matches number of required parameters. */
	UNE_UNPACK_RESULT_LIST(args, args_p, args_count);
	if (une_builtin_params_count(builtin) != args_count) {
		felix->error = UNE_ERROR_SET(UNE_ET_CALLABLE_ARG_COUNT, call->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	return (une_builtin_function_to_fnptr(builtin))(call, args_p+1);
}
