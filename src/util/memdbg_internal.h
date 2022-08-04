/*
memdbg_internal.h - Une
Modified 2022-08-04
*/

#ifndef MEMDBG_INTERNAL_H
#define MEMDBG_INTERNAL_H

/* Header-specific includes. */
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdbool.h>
#include <signal.h>
#ifndef _WIN32
#include <unistd.h>
#endif

// #define MEMDBG_SHOW_STATS
#define MEMDBG_PADDING_SIZE 32
#define MEMDBG_ALLOCATIONS_SIZE 4096

/*
Holds information for an allocation.
*/
typedef struct memdbg_allocation_ {
  char *file;
  int line;
  char *memory;
  size_t size;
  bool check_padding;
} memdbg_allocation;

/*
*** Interface.
*/

void *memdbg_malloc(char *file, int line, size_t size);
void *memdbg_calloc(char *file, int line, size_t count, size_t size);
void *memdbg_realloc(char *file, int line, void *memory, size_t size);
void memdbg_free(char *file, int line, void *memory);

void memdbg_allocations_padding_check__(char *file, int line);
void *memdbg_array_check__(char *file, int line, char *array, size_t array_size, size_t item_size, int index);
void memdbg_conclude__(void);
void memdbg_signal_handler__(int signum);

char *memdbg_strdup__(char *file, int line, char *str);
wchar_t *memdbg_wcsdup__(char *file, int line, wchar_t *wcs);
FILE *memdbg_fopen__(char *file, int line, char *path, char *mode);
int memdbg_fclose__(char *file, int line, FILE *fp);
void *memdbg_memset__(char *file, int line, void *str, int c, size_t n);

#endif /* !MEMDBG_INTERNAL_H */
