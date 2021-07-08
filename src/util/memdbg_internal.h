/*
memdbg_internal.h - Une
Modified 2021-07-08
*/

#ifndef MEMDBG_INTERNAL_H
#define MEMDBG_INTERNAL_H

/* Header-specific includes. */
#include <stddef.h>

#define MEMDBG_ALLOCATIONS_SIZE 1 // FIXME:

/*
Holds information for an allocation.
*/
typedef struct _memdbg_allocation {
  void *memory;
  char *file;
  int line;
  size_t size;
} memdbg_allocation;

/*
*** Interface.
*/

void *memdbg_malloc(size_t size, char *file, int line);
void *memdbg_realloc(void *memory, size_t size, char *file, int line);
void memdbg_free(void *memory, char *file, int line);

void __memdbg_conclude(void);

#endif /* !MEMDBG_INTERNAL_H */
