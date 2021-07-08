/*
memdbg.c - Une
Modified 2021-07-08
*/

/* Header-specific includes. */
#include "memdbg_internal.h"

/* Implementation-specific includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#define LOGGING_WIDE
#define LOGGING_ID "memdbg"
#include "logging.h"

/* Messages. */
#define MEMDBG_SUCCESS_ALL_FREED "All memory freed."
#define MEMDBG_WARN_SIZE_0 "Size is 0."
#define MEMDBG_WARN_FREE_NULLPTR "Freeing NULL pointer."
#define MEMDBG_WARN_NOT_FREED "Memory not freed."
#define MEMDBG_WARN_REALLOC_SIZE "Size smaller or same."
#define MEMDBG_WARN_ATEXIT "atexit failed. Call __memdbg_conclude manually."
#define MEMDBG_WARN_REALLOC_NULL "Reallocating NULL pointer."
#define MEMDBG_FAIL_OUT_OF_MEMORY "Out of memory."
#define MEMDBG_FAIL_UNOWNED_MEM "Unowned memory."

/* Globals. */
memdbg_allocation *memdbg_allocations = NULL;
size_t memdbg_allocations_size;
size_t memdbg_malloc_count;
size_t memdbg_realloc_count;
size_t memdbg_free_count;
size_t memdbg_allocations_count;

/* Private function declarations. */
static void memdbg_init(void);
static inline memdbg_allocation memdbg_allocation_reset(void);

/*
Allocates memory.
*/
void *memdbg_malloc(size_t size, char *file, int line)
{
  if (memdbg_allocations == NULL)
    memdbg_init();
  if (size == 0)
    warn_at(file, line, MEMDBG_WARN_SIZE_0);
  size_t index;
  for (index=0; index<memdbg_allocations_size; index++)
    if (memdbg_allocations[index].memory == NULL)
      break;
  if (index == memdbg_allocations_size) {
    memdbg_allocations_size *= 2;
    memdbg_allocations = realloc(memdbg_allocations, memdbg_allocations_size*sizeof(*memdbg_allocations));
  }
  void *memory = malloc(size);
  if (memory == NULL)
    fail_at(file, line, MEMDBG_FAIL_OUT_OF_MEMORY " (malloc %zu b)", size);
  memdbg_allocations[index] = (memdbg_allocation){
    .memory = memory,
    .file = file,
    .line = line,
    .size = size
  };
  memdbg_malloc_count++;
  memdbg_allocations_count++;
  return memory;
}

/*
Reallocates memory.
*/
void *memdbg_realloc(void *memory, size_t size, char *file, int line)
{
  if (memdbg_allocations == NULL)
    memdbg_init();
  if (memory == NULL) {
    warn_at(file, line, MEMDBG_WARN_REALLOC_NULL);
    return memdbg_malloc(size, file, line);
  }
  if (size == 0)
    warn_at(file, line, MEMDBG_WARN_SIZE_0);
  size_t index;
  for (index=0; index<memdbg_allocations_size; index++)
    if (memdbg_allocations[index].memory == memory)
      break;
  if (index == memdbg_allocations_size)
    fail_at(file, line, MEMDBG_FAIL_UNOWNED_MEM);
  if (size <= memdbg_allocations[index].size)
    warn_at(file, line, MEMDBG_WARN_REALLOC_SIZE);
  void *memory_new = realloc(memory, size);
  if (memory_new == NULL)
    fail_at(file, line, MEMDBG_FAIL_OUT_OF_MEMORY " (realloc %zu b)", size);
  memdbg_allocations[index].memory = memory_new;
  memdbg_allocations[index].size = size;
  memdbg_realloc_count++;
  return memory_new;
}

/*
Frees memory.
*/
void memdbg_free(void *memory, char *file, int line)
{
  if (memdbg_allocations == NULL)
    memdbg_init();
  if (memory == NULL)
    warn_at(file, line, MEMDBG_WARN_FREE_NULLPTR);
  size_t index;
  for (index=0; index<memdbg_allocations_size; index++)
    if (memdbg_allocations[index].memory == memory)
      break;
  if (index == memdbg_allocations_size)
    fail_at(file, line, MEMDBG_FAIL_UNOWNED_MEM);
  free(memory);
  memdbg_allocations[index] = memdbg_allocation_reset();
  memdbg_free_count++;
  memdbg_allocations_count--;
}

/*
Initializes the memory_allocations array.
*/
static void memdbg_init(void)
{
  memdbg_allocations_count = 0;
  memdbg_malloc_count = 0;
  memdbg_realloc_count = 0;
  memdbg_free_count = 0;
  assert(memdbg_allocations == NULL);
  memdbg_allocations_size = MEMDBG_ALLOCATIONS_SIZE;
  memdbg_allocations = malloc(memdbg_allocations_size*sizeof(*memdbg_allocations));
  if (memdbg_allocations == NULL)
    fail(MEMDBG_FAIL_OUT_OF_MEMORY " (memdbg_init: malloc %zu b)", memdbg_allocations_size*sizeof(*memdbg_allocations));
  for (size_t i=0; i<memdbg_allocations_size; i++)
    memdbg_allocations[i] = memdbg_allocation_reset();
  if (atexit(&__memdbg_conclude) != 0)
    warn(MEMDBG_WARN_ATEXIT);
}

/*
Reports unfreed memory and frees the memory_allocations array.
*/
void __memdbg_conclude(void)
{
  assert(memdbg_allocations_count == 0);
  assert(memdbg_malloc_count - memdbg_free_count == 0);
  bool all_memory_freed = true;
  if (memdbg_allocations != NULL) {
    for (size_t i=0; i<memdbg_allocations_size; i++)
      if (memdbg_allocations[i].memory != NULL) {
        all_memory_freed = false;
        warn_at(memdbg_allocations[i].file, memdbg_allocations[i].line, MEMDBG_WARN_NOT_FREED);
      }
    free(memdbg_allocations);
    memdbg_allocations = NULL;
  }
  if (all_memory_freed)
    success(MEMDBG_SUCCESS_ALL_FREED);
}

/*
Returns an empty memdbg_allocation.
*/
static inline memdbg_allocation memdbg_allocation_reset(void)
{
  return (memdbg_allocation){
    .memory = NULL,
    .file = NULL,
    .line = 0,
    .size = 0
  };
}
