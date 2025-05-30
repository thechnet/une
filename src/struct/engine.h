/*
engine.h - Une
Modified 2024-11-09
*/

#ifndef UNE_ENGINE_H
#define UNE_ENGINE_H

/* Header-specific includes. */
#include "../common.h"
#include "result.h"
#include "error.h"
#include "callable.h"
#include "interpreter_state.h"

/*
Macros.
*/

#define UNE_VERIFY_ENGINE assert(felix)

/*
A Une engine.
*/
typedef struct une_engine_ {
	une_error error;
	une_interpreter_state is;
} une_engine;

/*
Globals.
*/

extern une_engine *felix;

/*
*** Interface.
*/

une_engine une_engine_create_engine(void);
void une_engine_select_engine(une_engine *engine);
void une_engine_free(void);

void une_engine_prepare_for_next_module(void);
void une_engine_return_to_root_context(void);
une_module *une_engine_new_module_from_file_or_wcs(char *path, wchar_t *wcs);
une_callable *une_engine_parse_module(une_module *module);
une_result une_engine_interpret_file_or_wcs_with_position(char *path, wchar_t *wcs, une_position current_context_exit_position);
une_result une_engine_interpret_file_or_wcs(char *path, wchar_t *wcs);
void une_engine_print_error(void);
une_context *une_engine_push_context(bool transparent, une_position position_in_current_module, size_t child_module_id);
void une_engine_pop_context(une_context *context_before_push);
void une_engine_set_context_callable(une_callable *callable, wchar_t *optional_label);

#endif /* !UNE_ENGINE_H */
