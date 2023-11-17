/*
context.c - Une
Modified 2023-11-17
*/

/* Header-specific includes. */
#include "context.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Allocates, initializes, and returns a pointer to a marker une_context struct.
*/
une_context *une_context_create_marker(char *creation_file, une_position creation_point, wchar_t *callee_label, char *callee_definition_file, une_position callee_definition_point)
{
	/* Allocate une_context. */
	une_context *context = malloc(sizeof(*context));
	verify(context);
	
	/* Initialize une_context. */
	*context = (une_context){
		.is_marker_context = true,
		.parent = NULL,
		.creation_file = creation_file ? strdup(creation_file) : NULL,
		.creation_point = creation_point,
		.has_callee = false,
		.callee_label = callee_label ? wcsdup(callee_label) : NULL,
		.callee_definition_file = callee_definition_file ? strdup(callee_definition_file) : NULL,
		.callee_definition_point = callee_definition_point,
		.variables = NULL,
		.variables_size = UNE_SIZE_VARIABLE_BUF,
		.variables_count = 0
	};
	if (creation_file)
		verify(context->creation_file);
	if (callee_label)
		verify(callee_label);
	if (callee_definition_file)
		verify(context->callee_definition_file);
	
	return context;
}

/*
Allocates, initializes, and returns a pointer to a une_context struct.
*/
une_context *une_context_create(char *creation_file, une_position creation_point, bool has_callee, wchar_t *callee_label, char *callee_definition_file, une_position callee_definition_point)
{
	/* Create marker context. */
	une_context *context = une_context_create_marker(creation_file, creation_point, callee_label, callee_definition_file, callee_definition_point);
	
	/* Promote to full context. */
	context->is_marker_context = false;
	context->variables = malloc(UNE_SIZE_VARIABLE_BUF*sizeof(*context->variables));
	verify(context->variables);
	context->has_callee = has_callee;
	
	return context;
}

/*
Find and return the oldest parent of the incoming context.
*/
une_context *une_context_get_oldest_parent(une_context *context)
{
	while (context->parent != NULL)
		context = context->parent;
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
	if (context->creation_file)
		free(context->creation_file);
	if (context->callee_label)
		free(context->callee_label);
	if (context->callee_definition_file)
		free(context->callee_definition_file);
	
	/* Free une_association buffer. */
	if (!context->is_marker_context) {
		assert(context->variables);
		for (size_t i=0; i<context->variables_count; i++)
			une_association_free(context->variables[i]);
		free(context->variables);
	}
	
	free(context);
}

/*
Initializes a new une_association in a une_context's variable buffer.
*/
une_variable_itf__(une_variable_create)
{
	/* Find closest non-marker context. */
	while (context->is_marker_context) {
		assert(context->parent);
		context = context->parent;
	}
	
	/* Ensure sufficient space in une_association buffer. */
	assert(context->variables);
	if (context->variables_count >= context->variables_size) {
		context->variables_size *= 2;
		context->variables = realloc(context->variables, context->variables_size*sizeof(*context->variables));
		verify(context->variables);
	}
	
	/* Initialize une_association. */
	une_association *variable = une_association_create();
	variable->name = wcsdup(name);
	verify(variable->name);
	variable->content = une_result_create(UNE_RT_VOID); /* Don't use UNE_RT_none__ because this will be freed using une_result_free. */
	context->variables[context->variables_count++] = variable;
	
	return variable;
}

/*
Returns a pointer to a une_association in a une_context's variable buffer or NULL.
*/
une_variable_itf__(une_variable_find)
{
	/* Find une_association. */
	for (size_t i=0; i<context->variables_count; i++)
		if (wcscmp(context->variables[i]->name, name) == 0)
			return context->variables[i];

	/* Return NULL if no match was found. */
	return NULL;
}

/*
Returns a pointer to a une_association in a une_context's variable buffer and its parents or NULL.
*/
une_variable_itf__(une_variable_find_global)
{
	/* Return NULL by default. */
	une_association *var = NULL;
	
	/* Find une_association. */
	while (var == NULL) {
		var = une_variable_find(context, name);
		if (context->parent == NULL)
			break;
		context = context->parent;
	}
	
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
	return une_variable_create(une_context_get_oldest_parent(context), name);
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
