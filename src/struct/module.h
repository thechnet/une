/*
module.h - Une
Modified 2023-12-10
*/

#ifndef UNE_MODULE_H
#define UNE_MODULE_H

/* Header-specific includes. */
#include "../common.h"
#include "token.h"
#include "node.h"

/*
A module.
*/
typedef struct une_module_ {
	size_t id;
	bool originates_from_file;
	char *path;
	wchar_t *source;
	une_token *tokens;
} une_module;

/*
A module buffer.
*/
typedef struct une_modules_ {
	size_t size;
	une_module *buffer;
	size_t next_unused_id;
} une_modules;

/*
*** Interface.
*/

une_modules une_modules_create(void);
une_module *une_modules_add_module(une_modules *modules);
une_module *une_modules_get_module_by_id(une_modules modules, size_t id);
void une_modules_free(une_modules *modules);

#endif /* !UNE_MODULE_H */
