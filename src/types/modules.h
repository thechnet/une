/*
modules.h - Une
Modified 2023-11-18
*/

#ifndef UNE_MODULES_H
#define UNE_MODULES_H

/* Header-specific includes. */
#include "../primitive.h"
#include "token.h"
#include "node.h"

/*
FIXME: Desc.
*/
typedef struct une_module_ {
	size_t id;
	bool is_file; /* FIXME: Name. */
	char *path;
	wchar_t *source;
	une_token *tokens;
} une_module;

/*
FIXME: Desc.
*/
typedef struct une_modules_ {
	size_t size;
	une_module *buffer;
	size_t next_unused_id; /* FIXME: Name. */
} une_modules;

/*
*** Interface.
*/

une_modules une_modules_create(void);
une_module *une_modules_add_module(une_modules *modules);
une_module *une_modules_get_module_by_id(une_modules modules, size_t id);
void une_modules_free(une_modules *modules);

#endif /* !UNE_MODULES_H */
