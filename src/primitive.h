/*
primitive.h - Une
Updated 2021-04-17
*/

#ifndef UNE_PRIMITIVE_H
#define UNE_PRIMITIVE_H

#define UNE_DEBUG_MALLOC_COUNTER
#define UNE_DO_READ
#define UNE_DO_LEX
#define UNE_DO_PARSE
#define UNE_DO_INTERPRET

#include <stdlib.h>
#include <wchar.h>
#include <stdint.h>
#include <stdbool.h>

// IO Number Types
typedef int64_t une_int;
typedef double une_flt;

// TCC implements a nonportable swprintf.
#ifdef UNE_USE_NONPORTABLE_SWPRINTF
  #define swprintf(dest, size, format, ...) swprintf((dest), (format), ##__VA_ARGS__)
#endif

// malloc/free Counter for Debugging
#ifdef UNE_DEBUG_MALLOC_COUNTER
  extern int malloc_counter;
  #define malloc(size) malloc((size)); malloc_counter++;
  #define free(memory) { free((memory)); malloc_counter--; }
#endif

// Temporary Sizes
#define UNE_SIZE_SMALL 128
#define UNE_SIZE_MEDIUM 4096
#define UNE_SIZE_BIG 32767

// Output Color Escape Sequences
#define UNE_COLOR_SUCCESS L"\33[92m"
#define UNE_COLOR_FAIL L"\33[31m"
#define UNE_COLOR_NEUTRAL L"\33[0m"
#define UNE_COLOR_HINT L"\33[90m"
#define UNE_COLOR_TOKEN_TYPE L"\33[93m"
#define UNE_COLOR_TOKEN_VALUE L"\33[92m"
#define UNE_COLOR_NODE_BRANCH_TYPE L"\33[96m"
#define UNE_COLOR_NODE_DATUM_TYPE L"\33[95m"
#define UNE_COLOR_NODE_DATUM_VALUE L"\33[35m"

// Temporary Internal Error Response
#define WERR(msg) { wprintf(UNE_COLOR_FAIL L"Line %d: %ls\33[0m\n", __LINE__, msg); exit(1); }

#pragma region une_position
typedef struct _une_position {
  size_t start;
  size_t end;
} une_position;
#pragma endregion une_position

#pragma region une_value
typedef union _une_value {
  une_int _int;
  une_flt _flt;
  wchar_t *_wcs;
  void **_vpp;
  void *_vp;
} une_value;
#pragma endregion une_value

#endif /* !UNE_PRIMITIVE_H */