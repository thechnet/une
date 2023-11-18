/*
engine.h - Une
Modified 2023-11-18
*/

#ifndef UNE_ENGINE_H
#define UNE_ENGINE_H

/* Header-specific includes. */
#include "../primitive.h"
#include "error.h"
#include "interpreter_state.h"
#include "callable.h"

/*
FIXME: Desc.
*/
typedef struct une_engine_ {
	une_error error;
	une_interpreter_state is;
} une_engine;

/*
*** Interface.
*/

une_engine une_engine_create(void);
une_module *une_engine_new_module_from_file_or_wcs(une_engine *engine, char *path, wchar_t *wcs);
une_callable *une_engine_parse_module(une_engine *engine, une_module *module);
une_result une_engine_interpret_file_or_wcs(une_engine *engine, char *path, wchar_t *wcs);
void une_engine_print_error(une_engine *engine);
void une_engine_free(une_engine engine);

#endif /* !UNE_ENGINE_H */
