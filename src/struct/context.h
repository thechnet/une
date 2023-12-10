/*
context.h - Une
Modified 2023-12-10
*/

#ifndef UNE_CONTEXT_H
#define UNE_CONTEXT_H

/* Header-specific includes. */
#include "../common.h"
#include "association.h"

/*
Holds information that changes depending on the execution context.
*/
typedef struct une_context_ {
	struct une_context_ *parent;
	size_t module_id;
	size_t callable_id;
	une_position exit_position;
	bool is_transparent;
	struct {
		size_t size;
		size_t count;
		une_association **buffer;
	} variables;
} une_context;

/*
*** Interface.
*/

une_context *une_context_create_transparent(void);
une_context *une_context_create(void);

une_context *une_context_get_oldest_parent_or_self(une_context *context);
une_context *une_context_get_opaque_self_or_oldest_opaque_parent_or_null(une_context *context);
une_context *une_context_get_opaque_self_or_youngest_opaque_parent_or_null(une_context *context);

void une_context_free_children(une_context *parent, une_context *youngest_child);
void une_context_free(une_context *context);
une_context *une_context_stump(une_context *youngest_child);

size_t une_context_get_lineage(une_context *subject, une_context ***out);

une_association *une_variable_create(une_context *context, wchar_t *name);
une_association *une_variable_find_by_name(une_context *context, wchar_t *name);
une_association *une_variable_find_by_name_global(une_context *context, wchar_t *name);
une_association *une_variable_find_by_name_or_create(une_context *context, wchar_t *name);
une_association *une_variable_find_by_name_or_create_global(une_context *context, wchar_t *name);
une_association *une_variable_find_by_content_global(une_context *context, une_result content);

#endif /* !UNE_CONTEXT_H */
