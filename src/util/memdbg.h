/*
memdbg.h - Une
Modified 2021-07-08
*/

#ifndef MEMDBG_H
#define MEMDBG_H

#include "memdbg_internal.h"

/*
This is isolated to keep the memory functions in the
implementation from mapping to our new functions.
*/
#define malloc(size) memdbg_malloc(size, __FILE__, __LINE__)
#define realloc(memory, size) memdbg_realloc(memory, size, __FILE__, __LINE__)
#define free(memory) memdbg_free(memory, __FILE__, __LINE__)

#endif /* !MEMDBG_H */
