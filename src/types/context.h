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
	bool is_marker_context;
	struct une_context_ *parent;
	char *creation_file;
	une_position creation_point;
	bool has_callee;
	wchar_t *callee_label;
	char *callee_definition_file;
	une_position callee_definition_point;
	size_t variables_size;
	size_t variables_count;
	une_association **variables;
} une_context;

/*
*** Interface.
*/

/*
Variable interface function template.
*/
#define une_variable_itf__(name__) une_association *(name__)(une_context *context, wchar_t *name) 

une_context *une_context_create_marker(char *creation_file, une_position creation_point, wchar_t *callee_label, char *callee_definition_file, une_position callee_definition_point);
une_context *une_context_create(char *creation_file, une_position creation_point, bool has_callee, wchar_t *callee_label, char *callee_definition_file, une_position callee_definition_point);

une_context *une_context_get_oldest_parent_or_self(une_context *context);
une_context *une_context_get_oldest_nonmarker_parent_or_nonmarker_self(une_context *context);
une_context *une_context_get_youngest_nonmarker_parent_or_nonmarker_self(une_context *context);

void une_context_free_children(une_context *parent, une_context *youngest_child);
void une_context_free(une_context *context);

size_t une_context_get_lineage(une_context *subject, une_context ***out);

une_variable_itf__(une_variable_create);
une_variable_itf__(une_variable_find);
une_variable_itf__(une_variable_find_global);
une_variable_itf__(une_variable_find_or_create);
une_variable_itf__(une_variable_find_or_create_global);

#endif /* !UNE_CONTEXT_H */
