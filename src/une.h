/*
une.h - Une
Updated 2021-06-12
*/

#ifndef UNE_UNE_H
#define UNE_UNE_H

/* Header-specific includes. */
#include "types/result.h"

/*
*** Interface.
*/

une_result une_run(bool read_from_file, char *path, wchar_t *text);

#endif /* !UNE_UNE_H */
