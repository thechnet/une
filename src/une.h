/*
une.h - Une
Modified 2022-09-26
*/

#ifndef UNE_UNE_H
#define UNE_UNE_H

/* Header-specific includes. */
#include "types/result.h"
#include "types/interpreter_state.h"

/*
*** Interface.
*/

une_result une_run(bool read_from_file, char *path, wchar_t *text, bool *did_exit, une_interpreter_state *existing_interpreter_state);

#endif /* !UNE_UNE_H */
