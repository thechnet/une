/*
callable.h - Une
Modified 2023-11-20
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
	size_t module_id;
	une_position position;
	struct {
		size_t count;
		wchar_t **names;
	} parameters;
	une_node *body;
	bool is_module; // FIXME: We need this to decide if we free callable::body's strings when we destroy this struct, but that's not very elegant. Should we duplicate all the strings in the AST?
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
une_callable *une_callables_add_callable(une_callables *callables);
une_callable *une_callables_get_callable_by_id(une_callables callables, size_t id);
void une_callables_free(une_callables *callables);

#endif /* !UNE_CALLA_H */
