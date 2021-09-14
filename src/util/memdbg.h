/*
memdbg.h - Une
Modified 2021-08-22
*/

#ifndef MEMDBG_H
#define MEMDBG_H

#ifdef MEMDBG_ENABLE

#include "memdbg_internal.h"

/*
These are isolated to keep the memory functions in the
implementation from mapping to themselves.
*/
#define malloc(size) memdbg_malloc(__FILE__, __LINE__, size)
#define calloc(count, size) memdbg_calloc(__FILE__, __LINE__, count, size)
#define realloc(memory, size) memdbg_realloc(__FILE__, __LINE__, memory, size)
#define free(memory) memdbg_free(__FILE__, __LINE__, memory)

/* Manual functionality. */
#define padchk(memory) __memdbg_allocations_padding_check(__FILE__, __LINE__)
#define ARR(arr, idx)\
  (*(typeof(arr[0])*)__memdbg_array_check(__FILE__, __LINE__, (char*)arr, sizeof(arr), sizeof(arr[0]), idx))

/* Wrappers. */
#define strdup(str) __memdbg_strdup(__FILE__, __LINE__, str)
#define wcsdup(wcs) __memdbg_wcsdup(__FILE__, __LINE__, wcs)
#define fopen(path, mode) __memdbg_fopen(__FILE__, __LINE__, path, mode)
#define fclose(fp) __memdbg_fclose(__FILE__, __LINE__, fp)

#undef memset
#define memset(str, c, n) __memdbg_memset(__FILE__, __LINE__, str, c, n);

#else /* MEMDBG_ENABLE */

#define padchk(memory)
#define ARR(arr, idx) arr[idx]

#endif /* !MEMDBG_ENABLE */

#endif /* !MEMDBG_H */
