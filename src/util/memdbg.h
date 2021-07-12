/*
memdbg.h - Une
Modified 2021-07-11
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
#define strndup(str, n) __memdbg_strndup(__FILE__, __LINE__, str, n)
#define wcsdup(wcs) __memdbg_wcsdup(__FILE__, __LINE__, wcs)

#undef memset
#define memset(str, c, n) __memdbg_memset(__FILE__, __LINE__, str, c, n);

#else

#define padchk(memory)
#define ARR(arr, idx) arr[idx]

#endif

/*
(UNFINISHED, DO NOT USE) Regular expression to convert array notation to ARR() notation:
Find: \b((?<!\w\s)|return )((?:[_a-zA-Z]\w*(?:\s*(?:\.|->)\s*[_a-zA-Z]\w*)*\s*\[.+?\]\s*(?:\.|->)\s*)*)(?:([_a-zA-Z]\w*(?:\s*(?:\.|->)\s*[_a-zA-Z]\w*)*)\s*\[(.+?)\])
Replace: $1ARR($2$3, $4)
*/

#endif /* !MEMDBG_H */
