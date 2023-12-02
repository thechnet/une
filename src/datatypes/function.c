/*
function.c - Une
Modified 2023-12-02
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
Check if subject is equal to comparison.
*/
une_int une_datatype_function_is_equal(une_result subject, une_result comparison)
{
	assert(subject.type == UNE_RT_FUNCTION);
	if (comparison.type != UNE_RT_FUNCTION)
		return 0;
	return subject.value._id == comparison.value._id;
}

/*
Call result.
*/
une_result une_datatype_function_call(une_node *call, une_result function, une_result args, wchar_t *label)
{
	/* Get function. */
	assert(function.type == UNE_RT_FUNCTION);
	une_callable *callable = une_callables_get_callable_by_id(felix->is.callables, function.value._id);
	assert(callable);
	
	/* Ensure number of arguments matches number of required parameters. */
	UNE_UNPACK_RESULT_LIST(args, args_p, args_count);
	if (callable->parameters.count != args_count) {
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
	felix->is.context->callable_id = callable->id;

	/* Define parameters. */
	for (size_t i=0; i<callable->parameters.count; i++) {
		une_association *var = une_variable_create(felix->is.context, (callable->parameters.names)[i]);
		var->content = une_result_copy(args_p[i+1]);
	}

	/* Interpret body. */
	une_result result = une_interpret(callable->body);
	felix->is.should_return = false;

	/* Return to parent context. */
	if (result.type != UNE_RT_ERROR) {
		une_context_free_children(parent, felix->is.context);
		felix->is.context = parent;
	}
	return result;
}
