/*
context.c - Une
Modified 2023-12-01
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
	assert(context);
	while (context->parent != NULL)
		context = context->parent;
	return context;
}

/*
Find and return the oldest opaque parent (or self) of the incoming context, or NULL.
*/
une_context *une_context_get_opaque_self_or_oldest_opaque_parent_or_null(une_context *context)
{
	assert(context);
	une_context *opaque_self_or_oldest_opaque_parent_or_null = NULL;
	do {
		if (!context->is_transparent)
			opaque_self_or_oldest_opaque_parent_or_null = context;
		context = context->parent;
	} while (context);
	return opaque_self_or_oldest_opaque_parent_or_null;
}

/*
Find and return the youngest opaque parent (or self) of the incoming context, or NULL.
*/
une_context *une_context_get_opaque_self_or_youngest_opaque_parent_or_null(une_context *context)
{
	while (context) {
		if (!context->is_transparent)
			break;
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

/*
Find a variable.
*/
static une_association *une_variable_find_or_create_(une_context *starting_context, wchar_t *name, une_result content, bool global, bool create_if_not_found)
{
	assert(starting_context);
	assert(
		(name && content.type == UNE_RT_none__) ||
		(!name && UNE_RESULT_TYPE_IS_VALID(content.type) && !create_if_not_found)
	);

	/* Return NULL by default. */
	une_association *association = NULL;

	/* Search in the youngest opaque parent (or self). Assert that such a context exists. */
	une_context *search_context = une_context_get_opaque_self_or_youngest_opaque_parent_or_null(starting_context);
	assert(search_context);

	/* Find association. */
	do {
		for (size_t i=0; i<search_context->variables.count; i++) {
			if (
				(name && !wcscmp(search_context->variables.buffer[i]->name, name)) ||
				(
					une_result_equals_result(search_context->variables.buffer[i]->content, content)
				)
			) {
				association = search_context->variables.buffer[i];
				break;
			}
		}
		if (association || !global)
			break;
		search_context = une_context_get_opaque_self_or_youngest_opaque_parent_or_null(search_context->parent);
	} while (search_context);
	
	/* No association found, create it. */
	if (!association && create_if_not_found) {
		une_context *target_context = (
			global ?
				une_context_get_opaque_self_or_oldest_opaque_parent_or_null(starting_context)
			:
				une_context_get_opaque_self_or_youngest_opaque_parent_or_null(starting_context)
		);
		assert(target_context);

		/* Ensure there is sufficient space in the association buffer. */
		assert(target_context->variables.buffer);
		if (target_context->variables.count >= target_context->variables.size) {
			target_context->variables.size *= 2;
			target_context->variables.buffer = realloc(target_context->variables.buffer, target_context->variables.size*sizeof(*target_context->variables.buffer));
			verify(target_context->variables.buffer);
		}

		/* Initialize a new association. */
		association = une_association_create();
		association->name = wcsdup(name);
		verify(association->name);
		association->content = une_result_create(UNE_RT_VOID); /* Don't use UNE_RT_none__ because this will be freed using une_result_free. */
		target_context->variables.buffer[target_context->variables.count++] = association;
	}

	return association;
}

/*
Initializes a new une_association in a une_context's variable buffer.
*/
une_association *une_variable_create(une_context *context, wchar_t *name)
{
	/* Find closest opaque context. */
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
une_association *une_variable_find_by_name(une_context *context, wchar_t *name)
{
	return une_variable_find_or_create_(context, name, une_result_create(UNE_RT_none__), false, false);
}

/*
Returns a pointer to a une_association in a une_context's variable buffer and its parents or NULL.
*/
une_association *une_variable_find_by_name_global(une_context *context, wchar_t *name)
{
	return une_variable_find_or_create_(context, name, une_result_create(UNE_RT_none__), true, false);
}

/*
Returns a pointer to a une_association in a une_context's variable buffer or creates and initializes it.
*/
une_association *une_variable_find_by_name_or_create(une_context *context, wchar_t *name)
{
	return une_variable_find_or_create_(context, name, une_result_create(UNE_RT_none__), false, true);
}

/*
Returns a pointer to a une_association in a une_context's variable buffer and its parents or creates and initializes it.
*/
une_association *une_variable_find_by_name_or_create_global(une_context *context, wchar_t *name)
{
	return une_variable_find_or_create_(context, name, une_result_create(UNE_RT_none__), true, true);
}

/*
Returns a pointer to a une_association (found by content) in a une_context's variable buffer and its parents or NULL.
*/
une_association *une_variable_find_by_content_global(une_context *context, une_result content)
{
	return une_variable_find_or_create_(context, NULL, content, true, false);
}
