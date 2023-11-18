/*
interpreter_state.h - Une
Modified 2023-11-18
*/

#ifndef UNE_INTERPRETER_STATE_H
#define UNE_INTERPRETER_STATE_H

/* Header-specific includes. */
#include "../primitive.h"
#include "context.h"
#include "callable.h"
#include "modules.h"

/*
A temporary result buffer.
*/
typedef struct une_holding_ {
	une_result *buffer;
	size_t size;
	size_t count;
} une_holding;

/*
Holds the state of the interpreter.
*/
typedef struct une_interpreter_state_ {
	une_context *context;
	une_callables callables;
	une_modules modules;
	bool should_return;
	bool should_exit;
	une_result this_contestant;
	une_result this;
	une_holding holding;
} une_interpreter_state;

extern une_interpreter_state *une_is;

/*
*** Interface.
*/

une_interpreter_state une_interpreter_state_create(char *creation_file);
void une_interpreter_state_free(une_interpreter_state *is);
une_holding une_interpreter_state_holding_strip(void);
void une_interpreter_state_holding_reinstate(une_holding old);
une_result *une_interpreter_state_holding_add(une_result result);
void une_interpreter_state_holding_purge(void);
void une_interpreter_state_reset_flags(une_interpreter_state *is);
bool une_result_is_reference_to_foreign_object(une_result subject);

#endif /* UNE_INTERPRETER_STATE_H */
