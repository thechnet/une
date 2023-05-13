/*
une.h - Une
Modified 2023-05-13
*/

#ifndef UNE_UNE_H
#define UNE_UNE_H

/* Header-specific includes. */
#include "types/result.h"
#include "types/interpreter_state.h"
#include "types/error.h"

/*
*** Interface.
*/

une_result une_run(bool read_from_file, char *path, wchar_t *text, bool *did_exit, une_interpreter_state *existing_interpreter_state);
une_result une_run_bare(une_error *error, char *path, wchar_t *text);

#endif /* !UNE_UNE_H */
