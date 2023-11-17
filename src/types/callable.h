/*
callable.h - Une
Modified 2023-11-17
*/

// FIXME: Needs better name.

#ifndef UNE_CALLABLE_H
#define UNE_CALLABLE_H

/* Header-specific includes. */
#include "../primitive.h"
#include "node.h"

/*
A callable.
*/
typedef struct une_callable_ {
	size_t id;
	char *definition_file;
	une_position definition_point;
	size_t params_count;
	wchar_t **params;
	une_node *body;
} une_callable;

/*
A buffer of callables.
*/
typedef struct une_callables {
	une_callable *buffer;
	size_t size;
	size_t next_unused_id;
} une_callables;

/*
*** Interface.
*/

une_callables une_callables_create(void);
size_t une_callables_register_function(
	une_callables *callables,
	char *definition_file,
	une_position definition_point,
	size_t params_count,
	wchar_t **params,
	une_node *body);
une_callable *une_callables_get_callable_by_id(une_callables callables, size_t id);
void une_callables_free(une_callables *callables);

#endif /* !UNE_CALLA_H */
