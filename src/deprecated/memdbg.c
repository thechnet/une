/*
memdbg.c - Une
Modified 2025-07-26
*/

/* Header-specific includes. */
#include "memdbg_internal.h"

/* Implementation-specific includes. */
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <stdint.h>
#include <stddef.h>
#include "../../logging/escseq/include/escseq.h"
#define LOGGING_WIDE
#define LOGGING_ID "memdbg"
#include "../../logging/include/logging.h"

#define STDOUT_FILENO 1

/* Messages. */
#define MEMDBG_MSG_ALL_FREED "All memory freed."
#define MEMDBG_MSG_SIZE_0 "Size is 0."
#define MEMDBG_MSG_FREE_NULLPTR "Freeing NULL pointer."
#define MEMDBG_MSG_NOT_FREED "Memory not freed."
#define MEMDBG_MSG_REALLOC_SIZE "Size smaller or same."
#define MEMDBG_MSG_ATEXIT "atexit failed. Call memdbg_conclude__ manually."
#define MEMDBG_MSG_RECEIVING_NULL "Receiving NULL pointer."
#define MEMDBG_MSG_RETURNING_NULL "Returning NULL pointer."
#define MEMDBG_MSG_PADDING "Padding not intact."
#define MEMDBG_MSG_PADDING_AT "Padding not intact at " LOGGING_WHERE "."
#define MEMDBG_MSG_OUT_OF_MEMORY "Out of memory."
#define MEMDBG_MSG_UNOWNED_MEM "Unowned memory."
#define MEMDBG_MSG_INDEX_OUT_OF_RANGE "Index out of range. (%d of #%zu)"
#define MEMDBG_MSG_SIGNAL "signal failed. Not catching signals."
#define MEMDBG_MSG_SIGFPE "Arithmetic error."
#define MEMDBG_MSG_SIGILL "Illegal instruction."
#define MEMDBG_MSG_SIGSEGV "Segmentation fault."

/* Globals. */
char memdbg_padding[MEMDBG_PADDING_SIZE];
memdbg_allocation *memdbg_allocations = NULL;
uint64_t memdbg_allocations_size;
int64_t memdbg_allocations_count;
int64_t memdbg_malloc_count;
int64_t memdbg_realloc_count;
int64_t memdbg_free_count;
int64_t memdbg_alert_count;
int64_t memdbg_current_total_size;
int64_t memdbg_max_total_size;

/* Private Function Declarations. */
static void memdbg_init(void);
static inline void memdbg_allocation_add(memdbg_allocation allocation);
static inline void memdbg_allocation_remove(memdbg_allocation *allocation);
static inline size_t memdbg_allocation_find(void *memory);
static inline memdbg_allocation memdbg_allocation_reset(void);
static inline void memdbg_allocation_padding_set(memdbg_allocation allocation);
static inline void memdbg_allocation_padding_clear(memdbg_allocation allocation);
static inline int memdbg_current_second(void);

/****** Allocators. ******/

/*
Allocates memory.
*/
void *memdbg_malloc(char *file, int line, size_t size)
{
	if (memdbg_allocations == NULL)
		memdbg_init();
	memdbg_allocations_padding_check__(file, line);
	if (size == 0) {
		memdbg_alert_count++;
		warn_at(file, line, MEMDBG_MSG_SIZE_0);
	}
	void *memory = malloc(size+MEMDBG_PADDING_SIZE);
	if (memory == NULL) {
		memdbg_alert_count++;
		fail_at(file, line, MEMDBG_MSG_OUT_OF_MEMORY " (malloc %zu+%zu b)", size, MEMDBG_PADDING_SIZE);
	}
	memdbg_allocation_add((memdbg_allocation){
		.file = file,
		.line = line,
		.memory = memory,
		.size = size,
		.check_padding = true
	});
	return memory;
}

/*
Allocates memory and initializes it to 0.
*/
void *memdbg_calloc(char *file, int line, size_t count, size_t size)
{
	void *memory = memdbg_malloc(file, line, count*size);
	memset(memory, 0, count*size);
	return memory;
}

/*
Reallocates memory.
*/
void *memdbg_realloc(char *file, int line, void *memory, size_t size)
{
	if (memdbg_allocations == NULL)
		memdbg_init();
	memdbg_allocations_padding_check__(file, line);
	if (memory == NULL) {
		memdbg_alert_count++;
		warn_at(file, line, MEMDBG_MSG_RECEIVING_NULL);
		return memdbg_malloc(file, line, size);
	}
	if (size == 0) {
		memdbg_alert_count++;
		warn_at(file, line, MEMDBG_MSG_SIZE_0);
	}
	size_t index = memdbg_allocation_find(memory);
	if (index == memdbg_allocations_size) {
		memdbg_alert_count++;
		fail_at(file, line, MEMDBG_MSG_UNOWNED_MEM);
	}
	if (size <= memdbg_allocations[index].size) {
		memdbg_alert_count++;
		warn_at(file, line, MEMDBG_MSG_REALLOC_SIZE);
	}
	memdbg_allocation_padding_clear(memdbg_allocations[index]);
	void *memory_new = realloc(memory, size+MEMDBG_PADDING_SIZE);
	if (memory_new == NULL) {
		memdbg_alert_count++;
		fail_at(file, line, MEMDBG_MSG_OUT_OF_MEMORY " (realloc %zu+%zu b)", size, MEMDBG_PADDING_SIZE);
	}
	memdbg_current_total_size += size - memdbg_allocations[index].size;
	if (memdbg_current_total_size > memdbg_max_total_size)
		memdbg_max_total_size = memdbg_current_total_size;
	memdbg_allocations[index].memory = memory_new;
	memdbg_allocations[index].size = size;
	memdbg_allocation_padding_set(memdbg_allocations[index]);
	memdbg_realloc_count++;
	return memory_new;
}

/*
Frees memory.
*/
void memdbg_free(char *file, int line, void *memory)
{
	if (memdbg_allocations == NULL)
		memdbg_init();
	memdbg_allocations_padding_check__(file, line);
	if (memory == NULL) {
		memdbg_alert_count++;
		warn_at(file, line, MEMDBG_MSG_FREE_NULLPTR);
		return;
	}
	size_t index = memdbg_allocation_find(memory);
	if (index == memdbg_allocations_size) {
		memdbg_alert_count++;
		fail_at(file, line, MEMDBG_MSG_UNOWNED_MEM);
	}
	memdbg_allocation_padding_clear(memdbg_allocations[index]);
	memset(memory, 0, memdbg_allocations[index].size);
	free(memory);
	memdbg_allocation_remove(&memdbg_allocations[index]);
}

/****** Self-Initialization and Self-Destruction. ******/

/*
Initialize memdbg.
*/
static void memdbg_init(void)
{
	srand((unsigned int)memdbg_current_second());
	for (int i=0; i<MEMDBG_PADDING_SIZE; i++)
		memdbg_padding[i] = (char)(rand() % CHAR_MAX);
	assert(memdbg_allocations == NULL);
	memdbg_allocations_size = MEMDBG_ALLOCATIONS_SIZE;
	memdbg_allocations_count = 0;
	memdbg_malloc_count = 0;
	memdbg_realloc_count = 0;
	memdbg_free_count = 0;
	memdbg_current_total_size = 0;
	memdbg_max_total_size = 0;
	memdbg_allocations = malloc((size_t)memdbg_allocations_size*sizeof(*memdbg_allocations));
	if (memdbg_allocations == NULL) {
		memdbg_alert_count++;
		fail(MEMDBG_MSG_OUT_OF_MEMORY " (memdbg_init %zu b)", memdbg_allocations_size*sizeof(*memdbg_allocations));
	}
	for (size_t i=0; i<memdbg_allocations_size; i++)
		memdbg_allocations[i] = memdbg_allocation_reset();
	if (atexit(&memdbg_conclude__) != 0) {
		memdbg_alert_count++;
		warn(MEMDBG_MSG_ATEXIT);
	}
	if (signal(SIGFPE, &memdbg_signal_handler__) == SIG_ERR || signal(SIGILL, &memdbg_signal_handler__) == SIG_ERR || signal(SIGSEGV, &memdbg_signal_handler__) == SIG_ERR) {
		memdbg_alert_count++;
		warn(MEMDBG_MSG_SIGNAL);
	}
}

/*
Reports unfreed memory and frees the memory_allocations array.
*/
void memdbg_conclude__(void)
{
	bool all_memory_freed = true;
	if (memdbg_allocations != NULL) {
		memdbg_allocations_padding_check__(NULL, 0);
		for (size_t i=0; i<memdbg_allocations_size; i++)
			if (memdbg_allocations[i].memory != NULL) {
				all_memory_freed = false;
				memdbg_alert_count++;
				warn_at(memdbg_allocations[i].file, memdbg_allocations[i].line, MEMDBG_MSG_NOT_FREED);
			}
		free(memdbg_allocations);
		memdbg_allocations = NULL;
	}
	if (all_memory_freed) {
		assert(memdbg_allocations_count == 0);
		assert(memdbg_malloc_count - memdbg_free_count == 0);
		success(MEMDBG_MSG_ALL_FREED);
	}
	#ifdef MEMDBG_SHOW_STATS
	info("malloc/frees: %lld", memdbg_malloc_count);
	info("reallocs: %lld", memdbg_realloc_count);
	info("index size: %lld", memdbg_allocations_size);
	info("peak usage: %.3f kb", (double)memdbg_max_total_size/1000);
	info("alerts: %lld", memdbg_alert_count);
	#endif
}

/****** Checker Functions. ******/

/*
Verify that the padding of all allocations is intact.
*/
void memdbg_allocations_padding_check__(char *file, int line)
{
	if (memdbg_allocations == NULL)
		memdbg_init();
	for (size_t i=0; i<memdbg_allocations_size; i++)
		if (memdbg_allocations[i].memory != NULL && memdbg_allocations[i].check_padding)
			if (memcmp(memdbg_allocations[i].memory+memdbg_allocations[i].size, memdbg_padding, MEMDBG_PADDING_SIZE) != 0) {
				memdbg_allocations[i].check_padding = false;
				memdbg_alert_count++;
				if (file == NULL)
					warn_at(memdbg_allocations[i].file, memdbg_allocations[i].line, MEMDBG_MSG_PADDING);
				else
					warn_at(memdbg_allocations[i].file, memdbg_allocations[i].line, MEMDBG_MSG_PADDING_AT, file, line);
			}
}

/*
Check if 'idx' is a valid index into array 'arr'.
*/
void *memdbg_array_check__(char *file, int line, char *array, size_t array_size, size_t item_size, int index)
{
	if (array == NULL) {
		memdbg_alert_count++;
		fail_at(file, line, MEMDBG_MSG_RECEIVING_NULL);
	}
	assert(array_size != 0);
	assert(item_size != 0);
	bool is_stack_array = false;
	size_t allocations_index = memdbg_allocation_find(array);
	if (allocations_index == memdbg_allocations_size)
		is_stack_array = true;
	size_t items_count;
	if (is_stack_array)
		items_count = array_size/item_size;
	else
		items_count = memdbg_allocations[allocations_index].size/item_size;
	if (index < 0 || (size_t)index >= items_count) { // FIXME
		memdbg_alert_count++;
		fail_at(file, line, MEMDBG_MSG_INDEX_OUT_OF_RANGE, index, items_count);
	}
	return &array[(size_t)index*item_size];
}

/*
Handle signals.
*/
void __attribute__((noreturn)) memdbg_signal_handler__(int signum)
{
	#ifndef _WIN32
	char *msg;
	switch (signum) {
		case SIGFPE:
			msg = LOGGING_STYLE_FAIL__ LOGGING_ID__ MEMDBG_MSG_SIGFPE RESET "\n";
			break;
		case SIGILL:
			msg = LOGGING_STYLE_FAIL__ LOGGING_ID__ MEMDBG_MSG_SIGILL RESET "\n";
			break;
		case SIGSEGV:
			msg = LOGGING_STYLE_FAIL__ LOGGING_ID__ MEMDBG_MSG_SIGSEGV RESET "\n";
			break;
		default:
			assert(false);
	}
	write(STDOUT_FILENO, msg, strlen(msg)+1);
	#endif
	abort();
}


/****** Inline Functions. ******/

/*
Returns an empty memdbg_allocation.
*/
static inline memdbg_allocation memdbg_allocation_reset(void)
{
	return (memdbg_allocation){
		.file = NULL,
		.line = 0,
		.memory = NULL,
		.size = 0,
		.check_padding = true
	};
}

/*
Set the padding of a memdbg_allocation.
*/
static inline void memdbg_allocation_padding_set(memdbg_allocation allocation)
{
	memcpy(allocation.memory+allocation.size, memdbg_padding, MEMDBG_PADDING_SIZE);
}

/*
Clear the padding of a memdbg_allocation.
*/
static inline void memdbg_allocation_padding_clear(memdbg_allocation allocation)
{
	memset(allocation.memory+allocation.size, 0, MEMDBG_PADDING_SIZE);
}

/*
Find a pointer in the allocations index.
*/
static size_t memdbg_allocation_find(void *memory)
{
	size_t index;
	for (index=0; index<memdbg_allocations_size; index++)
		if (memdbg_allocations[index].memory == memory)
			break;
	return index;
}

/*
Add a memdbg_allocation to the index.
*/
static inline void memdbg_allocation_add(memdbg_allocation allocation)
{
	size_t index = memdbg_allocation_find(NULL);
	if (index == memdbg_allocations_size) {
		memdbg_allocations_size *= 2;
		memdbg_allocations = realloc(memdbg_allocations, (size_t)memdbg_allocations_size*sizeof(*memdbg_allocations));
		if (memdbg_allocations == NULL) {
			memdbg_alert_count++;
			fail(MEMDBG_MSG_OUT_OF_MEMORY " (memdbg_allocation_add %zu b)", memdbg_allocations_size*sizeof(*memdbg_allocations));
		}
		for (size_t i=index; i<memdbg_allocations_size; i++)
			memdbg_allocations[i] = memdbg_allocation_reset();
	}
	memdbg_allocations[index] = allocation;
	if (memdbg_allocations[index].check_padding)
		memdbg_allocation_padding_set(memdbg_allocations[index]);
	memdbg_current_total_size += memdbg_allocations[index].size;
	if (memdbg_current_total_size > memdbg_max_total_size)
		memdbg_max_total_size = memdbg_current_total_size;
	memdbg_allocations_count++;
	memdbg_malloc_count++;
}

/*
Remove a memdbg_allocation from the index.
*/
static inline void memdbg_allocation_remove(memdbg_allocation *allocation)
{
	memdbg_current_total_size -= allocation->size;
	*allocation = memdbg_allocation_reset();
	memdbg_free_count++;
	memdbg_allocations_count--;
}

/*
Get the current second.
*/
static inline int memdbg_current_second(void)
{
	time_t time_now = time(NULL);
	return localtime(&time_now)->tm_sec;
}

/*
*** Wrappers.
*/

#define memdbg_wrap_allocator(type_, name_, params_, memory_, call_, newsize_, check_incoming_memory_, enable_padding_)\
	type_ memdbg_##name_##__ params_\
	{\
		if (memdbg_allocations == NULL)\
			memdbg_init();\
		memdbg_allocations_padding_check__(file, line);\
		if (check_incoming_memory_ && memory_ == NULL)\
			info_at(file, line, MEMDBG_MSG_RECEIVING_NULL);\
		if (check_incoming_memory_ && memdbg_allocation_find(memory_) == memdbg_allocations_size)\
			info_at(file, line, MEMDBG_MSG_UNOWNED_MEM);\
		type_ memory_new = call_;\
		if (memory_new == NULL) {\
			info_at(file, line, MEMDBG_MSG_RETURNING_NULL);\
			return memory_new;\
		}\
		size_t size = (newsize_)*sizeof(*memory_new);\
		if (enable_padding_) {\
			memory_new = realloc(memory_new, size+MEMDBG_PADDING_SIZE);\
			if (memory_new == NULL) {\
				memdbg_alert_count++;\
				fail(MEMDBG_MSG_OUT_OF_MEMORY " (" #name_ " %zu+%zu b)", size, MEMDBG_PADDING_SIZE);\
			}\
		}\
		memdbg_allocation_add((memdbg_allocation){\
			.memory = (char*)memory_new,\
			.size = size,\
			.file = file,\
			.line = line,\
			.check_padding = enable_padding_\
		});\
		return memory_new;\
	}

#define memdbg_wrap_deallocator(type_, name_, params_, memory_, call_, enable_padding_)\
	type_ memdbg_##name_##__ params_\
	{\
		if (memdbg_allocations == NULL)\
			memdbg_init();\
		memdbg_allocations_padding_check__(file, line);\
		if (memory_ == NULL)\
			info_at(file, line, MEMDBG_MSG_RECEIVING_NULL);\
		size_t index = memdbg_allocation_find(memory_);\
		if (index == memdbg_allocations_size)\
			info_at(file, line, MEMDBG_MSG_UNOWNED_MEM);\
		else if (enable_padding_)\
			memdbg_allocation_padding_clear(memdbg_allocations[index]);\
		type_ return_value = call_;\
		if (index != memdbg_allocations_size)\
			memdbg_allocation_remove(&memdbg_allocations[index]);\
		return return_value;\
	}

#define memdbg_wrap_function(type_, name_, params_, memory_, call_)\
	type_ name_ params_\
	{\
		if (memdbg_allocations == NULL)\
			memdbg_init();\
		if (memory_ == NULL)\
			info_at(file, line, MEMDBG_MSG_RECEIVING_NULL);\
		if (memdbg_allocation_find(memory_) == memdbg_allocations_size)\
			info_at(file, line, MEMDBG_MSG_UNOWNED_MEM);\
		type_ return_value = call_;\
		memdbg_allocations_padding_check__(file, line);\
		return return_value;\
	}

memdbg_wrap_allocator\
	(char*, strdup, (char *file, int line, char *str),\
	str, strdup(str), strlen(memory_new)+1, false, true)

memdbg_wrap_allocator\
	(wchar_t*, wcsdup, (char *file, int line, wchar_t *wcs),\
	wcs, wcsdup(wcs), wcslen(memory_new)+1, true, true)

memdbg_wrap_allocator\
	(FILE*, fopen, (char *file, int line, char *path, char *mode),\
	path, fopen(path, mode), 1, false, false)

memdbg_wrap_deallocator\
	(int, fclose, (char *file, int line, FILE *fp),\
	fp, fclose(fp), false)

memdbg_wrap_function\
	(void*, memdbg_memset__, (char *file, int line, void *str, int c, size_t n),\
	str, memset(str, c, n))
