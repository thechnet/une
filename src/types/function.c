/*
function.c - Une
Modified 2024-11-09
*/

/* Header-specific includes. */
#include "function.h"

/* Implementation-specific includes. */
#include "../struct/association.h"
#include "../struct/callable.h"
#include "../interpreter.h"
#include "../tools.h"

/*
Print a text representation to file.
*/
void une_type_function_represent(FILE *file, une_result result)
{
	assert(result.kind == UNE_RK_FUNCTION);
	fwprintf(file, L"<function>");
}

/*
Check for truth.
*/
une_int une_type_function_is_true(une_result result)
{
	assert(result.kind == UNE_RK_FUNCTION);
	return 1;
}

/*
Check if subject is equal to comparison.
*/
une_int une_type_function_is_equal(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_FUNCTION);
	if (comparison.kind != UNE_RK_FUNCTION)
		return 0;
	return subject.value._id == comparison.value._id;
}

/*
Call result.
*/
une_result une_type_function_call(une_node *call, une_result function, une_result args, wchar_t *label)
{
	/* Get function. */
	assert(function.kind == UNE_RK_FUNCTION);
	une_callable *callable = une_callables_get_callable_by_id(felix->is.callables, function.value._id);
	assert(callable);
	
	/* Ensure number of arguments matches number of required parameters. */
	UNE_UNPACK_RESULT_LIST(args, args_p, args_count);
	if (callable->parameters.count != args_count) {
		felix->error = UNE_ERROR_SET(UNE_EK_CALLABLE_ARG_COUNT, call->pos);
		return une_result_create(UNE_RK_ERROR);
	}
	
	/* Push function context. */
	une_context *parent = une_engine_push_context(false, call->pos, callable->module_id);
	une_engine_set_context_callable(callable, label);

	/* Define parameters. */
	for (size_t i=0; i<callable->parameters.count; i++) {
		une_association *var = une_variable_create(felix->is.context, (callable->parameters.names)[i]);
		var->content = une_result_copy(args_p[i+1]);
	}

	/* Interpret body. */
	une_result result = une_interpret(callable->body);
	felix->is.should_return = false;

	/* Return to parent context. */
	if (result.kind != UNE_RK_ERROR) {
		une_engine_pop_context(parent);
	}
	return result;
}
