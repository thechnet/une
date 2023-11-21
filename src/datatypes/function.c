/*
function.c - Une
Modified 2023-11-21
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
	assert(callee);
	
	/* Ensure number of arguments matches number of required parameters. */
	UNE_UNPACK_RESULT_LIST(args, args_p, args_count);
	if (callee->parameters.count != args_count) {
		felix->error = UNE_ERROR_SET(UNE_ET_CALLABLE_ARG_COUNT, call->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Create function context. */
	une_context *parent = felix->is.context;
	felix->is.context = une_context_create();
	felix->is.context->parent = parent;
	une_callable *parent_callable = une_callables_get_callable_by_id(felix->is.callables, parent->callable_id);
	assert(parent_callable);
	felix->is.context->creation_module_id = parent_callable->module_id;
	felix->is.context->creation_position = call->pos;
	felix->is.context->callable_id = callee->id;

	/* Define parameters. */
	for (size_t i=0; i<callee->parameters.count; i++) {
		une_association *var = une_variable_create(felix->is.context, (callee->parameters.names)[i]);
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
