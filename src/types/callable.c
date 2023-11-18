/*
callable.c - Une
Modified 2023-11-18
*/

/* Header-specific includes. */
#include "callable.h"

/* Implementation-specific includes. */
#include "../tools.h"

static void une_callable_clear(une_callable *callable)
{
	assert(callable);

	if (callable->id == 0)
		return;
	
	if (callable->definition_file)
		free(callable->definition_file);
	
	if (callable->params_count > 0) {
		assert(callable->params);
		for (size_t i=0; i<callable->params_count; i++) {
			assert(callable->params[i]);
			free(callable->params[i]);
		}
		free(callable->params);
	}
	
	if (callable->body)
		une_node_free(callable->body, true);
	
	callable->id = 0;
}

une_callables une_callables_create(void)
{
	size_t size = UNE_SIZE_CALLABLES;
	
	une_callable *buffer = malloc(size*sizeof(*buffer));
	verify(buffer);
	for (size_t i=0; i<size; i++)
		buffer[i] = (une_callable){ .id = 0 };
	
	return (une_callables){
		.buffer = buffer,
		.size = size,
		.next_unused_id = 1
	};
}

une_callable *une_callables_add_callable(une_callables *callables)
{
	assert(callables);
	assert(callables->buffer);
	
	size_t index;
	for (index=0; index<callables->size; index++) {
		if (callables->buffer[index].id == 0)
			break;
	}
	if (index == callables->size) {
		callables->size *= 2;
		callables->buffer = realloc(callables->buffer, callables->size*sizeof(*callables->buffer));
		verify(callables->buffer);
		for (size_t i=index; i<callables->size; i++)
			callables->buffer[i] = (une_callable){ .id = 0 };
	}
	
	callables->buffer[index] = (une_callable){
		.id = callables->next_unused_id++
	};

	return callables->buffer+index;
}

une_callable *une_callables_get_callable_by_id(une_callables callables, size_t id)
{
	assert(callables.buffer);

	for (size_t i=0; i<callables.size; i++) {
		if (callables.buffer[i].id == id)
			return callables.buffer+i;
	}

	return NULL;
}

void une_callables_free(une_callables *callables)
{
	assert(callables);

	for (size_t i=0; i<callables->size; i++)
		une_callable_clear(callables->buffer+i);
	free(callables->buffer);

	callables->size = 0;
	callables->next_unused_id = 0;
}

