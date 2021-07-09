/*
memdbg.h - Une
Modified 2021-07-09
*/

#ifndef MEMDBG_H
#define MEMDBG_H

#include "memdbg_internal.h"

/*
These are isolated to keep the memory functions in the
implementation from mapping to themselves.
*/
#define malloc(size) memdbg_malloc(__FILE__, __LINE__, size)
#define calloc(count, size) memdbg_calloc(__FILE__, __LINE__, count, size)
#define realloc(memory, size) memdbg_realloc(__FILE__, __LINE__, memory, size)
#define free(memory) memdbg_free(__FILE__, __LINE__, memory)

#define padchk(memory) __memdbg_allocations_padding_check(__FILE__, __LINE__)

/* Wrappers. */
#define strdup(str) __memdbg_strdup(__FILE__, __LINE__, str)
#define strndup(str, n) __memdbg_strndup(__FILE__, __LINE__, str, n)
#define wcsdup(wcs) __memdbg_wcsdup(__FILE__, __LINE__, wcs)

#undef memset
#define memset(str, c, n) __memdbg_memset(__FILE__, __LINE__, str, c, n);

#endif /* !MEMDBG_H */
