/*
context.h - Une
Modified 2023-11-20
*/

#ifndef UNE_CONTEXT_H
#define UNE_CONTEXT_H

/* Header-specific includes. */
#include "../primitive.h"
#include "association.h"

/*
Holds information that changes depending on the execution context.
*/
typedef struct une_context_ {
	struct une_context_ *parent;
	bool is_transparent;
	struct {
		size_t size;
		size_t count;
		une_association **buffer;
	} variables;
	size_t creation_module_id;
	une_position creation_position;
	size_t callable_id;
} une_context;

/*
*** Interface.
*/

/*
Variable interface function template.
*/
#define une_variable_itf__(name__) une_association *(name__)(une_context *context, wchar_t *name) 

une_context *une_context_create_transparent(void);
une_context *une_context_create(void);

une_context *une_context_get_oldest_parent_or_self(une_context *context);
une_context *une_context_get_oldest_opaque_parent_or_opaque_self(une_context *context);
une_context *une_context_get_youngest_opaque_parent_or_opaque_self(une_context *context);

void une_context_free_children(une_context *parent, une_context *youngest_child);
void une_context_free(une_context *context);

// size_t une_context_get_lineage(une_context *subject, une_context ***out);

une_variable_itf__(une_variable_create);
une_variable_itf__(une_variable_find);
une_variable_itf__(une_variable_find_global);
une_variable_itf__(une_variable_find_or_create);
une_variable_itf__(une_variable_find_or_create_global);

#endif /* !UNE_CONTEXT_H */
