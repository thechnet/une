/*
context.c - Une
Modified 2023-11-20
*/

/* Header-specific includes. */
#include "context.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Allocates, initializes, and returns a pointer to a transparent une_context struct.
*/
une_context *une_context_create_transparent(void)
{
	/* Allocate une_context. */
	une_context *context = malloc(sizeof(*context));
	verify(context);
	
	/* Initialize une_context. */
	*context = (une_context){
		.is_transparent = true
	};
	
	return context;
}

/*
Allocates, initializes, and returns a pointer to a une_context struct.
*/
une_context *une_context_create(void)
{
	/* Create transparent context. */
	une_context *context = une_context_create_transparent();
	
	/* Promote to full context. */
	context->is_transparent = false;
	context->variables.size = UNE_SIZE_VARIABLE_BUF;
	context->variables.count = 0;
	context->variables.buffer = malloc(context->variables.size*sizeof(*context->variables.buffer));
	verify(context->variables.buffer);
	
	return context;
}

/*
Find and return the oldest parent of the incoming context.
*/
une_context *une_context_get_oldest_parent_or_self(une_context *context)
{
	while (context->parent != NULL)
		context = context->parent;
	return context;
}

/*
Find and return the oldest non-transparent parent (or self) of the incoming context, or NULL.
*/
une_context *une_context_get_oldest_opaque_parent_or_opaque_self(une_context *context)
{
	une_context *oldest_opaque_parent_or_opaque_self = NULL;
	do {
		if (!context->is_transparent)
			oldest_opaque_parent_or_opaque_self = context;
		context = context->parent;
	} while (context->parent != NULL);
	return oldest_opaque_parent_or_opaque_self;
}

/*
Find and return the youngest non-transparent parent (or self) of the incoming context, or NULL.
*/
une_context *une_context_get_youngest_opaque_parent_or_opaque_self(une_context *context)
{
	while (context) {
		if (!context->is_transparent) {
			break;
		}
		context = context->parent;
	}
	return context;
}

/*
Frees all une_contexts, starting at youngest_child and up to, but not including, parent.
*/
void une_context_free_children(une_context *parent, une_context *youngest_child)
{
	une_context *context = youngest_child;
	while (context != parent) {
		une_context *older_context = context->parent;
		une_context_free(context);
		context = older_context;
	}
}

/*
Frees a une_context and its owned members.
*/
void une_context_free(une_context *context)
{
	/* Free une_association buffer. */
	if (!context->is_transparent) {
		assert(context->variables.buffer);
		for (size_t i=0; i<context->variables.count; i++)
			une_association_free(context->variables.buffer[i]);
		free(context->variables.buffer);
	}
	
	free(context);
}

/*
Initializes a new une_association in a une_context's variable buffer.
*/
une_variable_itf__(une_variable_create)
{
	/* Find closest non-transparent context. */
	while (context->is_transparent) {
		assert(context->parent);
		context = context->parent;
	}
	
	/* Ensure sufficient space in une_association buffer. */
	assert(context->variables.buffer);
	if (context->variables.count >= context->variables.size) {
		context->variables.size *= 2;
		context->variables.buffer = realloc(context->variables.buffer, context->variables.size*sizeof(*context->variables.buffer));
		verify(context->variables.buffer);
	}
	
	/* Initialize une_association. */
	une_association *variable = une_association_create();
	variable->name = wcsdup(name);
	verify(variable->name);
	variable->content = une_result_create(UNE_RT_VOID); /* Don't use UNE_RT_none__ because this will be freed using une_result_free. */
	context->variables.buffer[context->variables.count++] = variable;
	
	return variable;
}

/*
Returns a pointer to a une_association in a une_context's variable buffer or NULL.
*/
une_variable_itf__(une_variable_find)
{
	/* Search in the youngest non-transparent parent (or self). */
	une_context *opaque_context = une_context_get_youngest_opaque_parent_or_opaque_self(context);
	assert(opaque_context);

	/* Find une_association. */
	for (size_t i=0; i<opaque_context->variables.count; i++)
		if (wcscmp(opaque_context->variables.buffer[i]->name, name) == 0)
			return opaque_context->variables.buffer[i];

	/* Return NULL if no match was found. */
	return NULL;
}

/*
Returns a pointer to a une_association in a une_context's variable buffer and its parents or NULL.
*/
une_variable_itf__(une_variable_find_global)
{
	assert(context);

	/* Return NULL by default. */
	une_association *var = NULL;
	
	/* Find une_association. */
	do {
		var = une_variable_find(context, name);
		if (var || !context->parent)
			break;
		context = une_context_get_youngest_opaque_parent_or_opaque_self(context->parent);
	} while (true);
	
	return var;
}

/*
Returns a pointer to a une_association in a une_context's variable buffer or creates and initializes it.
*/
une_variable_itf__(une_variable_find_or_create)
{
	/* Find une_association. */
	une_association *variable = une_variable_find(context, name);
	if (variable != NULL)
		return variable;
	
	/* une_association doesn't exist yet, create it. */
	return une_variable_create(context, name);
}

/*
Returns a pointer to a une_association in a une_context's variable buffer and its parents or creates and initializes it.
*/
une_variable_itf__(une_variable_find_or_create_global)
{
	/* Find une_association. */
	une_association *variable = une_variable_find_global(context, name);
	if (variable != NULL)
		return variable;
	
	/* une_association doesn't exist yet, create it in the oldest parent context. */
	return une_variable_create(une_context_get_oldest_parent_or_self(context), name);
}

/*
Trace a context's lineage, ending with the root. Return the number of elements in the list.
*/
size_t une_context_get_lineage(une_context *subject, une_context ***out)
{
	size_t contexts_size = UNE_SIZE_EXPECTED_TRACEBACK_DEPTH;
	une_context **contexts = malloc(contexts_size*sizeof(*contexts));
	verify(contexts);
	
	size_t contexts_length = 0;
	une_context *context = subject;
	while (true) {
		if (contexts_length >= UNE_SIZE_EXPECTED_TRACEBACK_DEPTH) {
			contexts_size *= 2;
			contexts = realloc(contexts, contexts_size*sizeof(*contexts));
			verify(contexts);
		}
		contexts[contexts_length++] = context;
		context = context->parent;
		if (!context)
			break;
	}
	
	*out = contexts;
	return contexts_length;
}
