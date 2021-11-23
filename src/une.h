/*
une.h - Une
Modified 2021-11-23
*/

#ifndef UNE_UNE_H
#define UNE_UNE_H

/* Header-specific includes. */
#include "types/result.h"
#include "types/context.h"

/*
*** Interface.
*/

une_result une_run(bool read_from_file, char *path, wchar_t *text, une_context *external_context);

#endif /* !UNE_UNE_H */
