/*
function.c - Une
Modified 2023-11-19
*/

/* Header-specific includes. */
#include "function.h"

/* Implementation-specific includes. */
#include "../types/association.h"
#include "../types/callable.h"
#include "../interpreter.h"
#include "../tools.h"

/*
Print a text representation to file.
*/
void une_datatype_function_represent(FILE *file, une_result result)
{
	assert(result.type == UNE_RT_FUNCTION);
	fwprintf(file, L"FUNCTION");
}

/*
Check for truth.
*/
une_int une_datatype_function_is_true(une_result result)
{
	assert(result.type == UNE_RT_FUNCTION);
	return 1;
}

/*
Call result.
*/
une_result une_datatype_function_call(une_node *call, une_result function, une_result args, wchar_t *label)
{
	/* Get function. */
	assert(function.type == UNE_RT_FUNCTION);
	une_callable *callee = une_callables_get_callable_by_id(felix->is.callables, function.value._id);
	
	/* Ensure number of arguments matches number of required parameters. */
	UNE_UNPACK_RESULT_LIST(args, args_p, args_count);
	if (callee->params_count != args_count) {
		felix->error = UNE_ERROR_SET(UNE_ET_CALLABLE_ARG_COUNT, call->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Create function context. */
	une_context *parent = felix->is.context;
	felix->is.context = une_context_create(felix->is.context->creation_file, call->pos, true, label, callee->definition_file, callee->definition_point);
	felix->is.context->parent = parent;

	/* Define parameters. */
	for (size_t i=0; i<callee->params_count; i++) {
		une_association *var = une_variable_create(felix->is.context, (callee->params)[i]);
		var->content = une_result_copy(args_p[i+1]);
	}

	/* Interpret body. */
	une_result result = une_interpret(callee->body);
	felix->is.should_return = false;

	/* Return to parent context. */
	if (result.type != UNE_RT_ERROR) {
		une_context_free_children(parent, felix->is.context);
		felix->is.context = parent;
	}
	return result;
}
