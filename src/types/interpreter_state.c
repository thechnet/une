/*
interpreter_state.c - Une
Modified 2023-11-17
*/

/* Header-specific includes. */
#include "interpreter_state.h"
#include "../tools.h"

/* Implementation-specific includes. */
#include "../datatypes/object.h"

une_interpreter_state *une_is = NULL;

/*
Initialize a une_interpreter_state struct.
*/
une_interpreter_state une_interpreter_state_create(char *creation_file)
{
	return (une_interpreter_state){
		.context = une_context_create(creation_file, (une_position){0}, false, NULL, NULL, (une_position){0}),
		.should_return = false,
		.should_exit = false,
		.this_contestant = une_result_create(UNE_RT_VOID),
		.this = une_result_create(UNE_RT_VOID),
		.holding = (une_holding){
			.buffer = NULL,
			.size = 0,
			.count = 0
		}
	};
}

/*
Free all members of a une_interpreter_state struct.
*/
void une_interpreter_state_free(une_interpreter_state *is)
{
	/* Context. */
	une_context_free_children(NULL, une_is->context);
	
	/* 'this' contestant. */
	une_result_free(une_is->this_contestant);
}

/*
Strip the current holding, replacing it with a new one.
*/
une_holding une_interpreter_state_holding_strip(void)
{
	/* Preserve current holding. */
	une_holding old = une_is->holding;
	
	/* Replace holding with new one. */
	une_result *buffer = malloc(UNE_SIZE_HOLDING*sizeof(*buffer));
	verify(buffer);
	for (size_t i=0; i<UNE_SIZE_HOLDING; i++)
		buffer[i] = une_result_create(UNE_RT_none__);
	une_is->holding = (une_holding){
		.buffer = buffer,
		.size = UNE_SIZE_HOLDING,
		.count = 0
	};
	
	/* Return old holding. */
	return old;
}

/*
Drop all held results.
*/
void une_interpreter_state_holding_reinstate(une_holding old)
{
	/* Empty current holding's buffer. */
	une_interpreter_state_holding_purge();
	free(une_is->holding.buffer);
	
	/* Reinstate old holding. */
	une_is->holding = old;
}

/*
Hold a result in the interpreter state.
*/
une_result *une_interpreter_state_holding_add(une_result result)
{
	/* Ensure buffer is big enough. */
	if (une_is->holding.count >= une_is->holding.size) {
		une_is->holding.size *= 2;
		une_is->holding.buffer = realloc(une_is->holding.buffer, une_is->holding.size*sizeof(*une_is->holding.buffer));
		verify(une_is->holding.buffer);
		for (size_t i=une_is->holding.count; i<une_is->holding.size; i++)
			une_is->holding.buffer[i] = une_result_create(UNE_RT_none__);
	}
	
	/* Hold result. */
	une_is->holding.buffer[une_is->holding.count++] = result;
	
	/* Return pointer to held result. */
	return une_is->holding.buffer+une_is->holding.count-1;
}

/*
Free the contents of a holding, without freeing the holding itself.
*/
void une_interpreter_state_holding_purge(void)
{
	for (size_t i=0; i<une_is->holding.count; i++) {
		une_result_free(une_is->holding.buffer[i]);
		une_is->holding.buffer[i] = une_result_create(UNE_RT_none__);
	}
	une_is->holding.count = 0;
}

/*
Check if a result matches the interpreter state's 'this'.
*/
bool une_result_is_reference_to_foreign_object(une_result subject)
{
	if (subject.type != UNE_RT_REFERENCE || subject.reference.type != UNE_FT_SINGLE)
		return false;
	une_result *referenced = (une_result*)subject.reference.root;
	assert(referenced);
	if (referenced->type != UNE_RT_OBJECT)
		return false;
	une_object *object = (une_object*)referenced->value._vp;
	assert(object);
	return object->owner != une_is->context;
}
