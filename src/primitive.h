/*
primitive.h - Une
Updated 2021-04-29
*/

#ifndef UNE_PRIMITIVE_H
#define UNE_PRIMITIVE_H

#define UNE_DEBUG
#define UNE_DO_READ
#define UNE_DO_LEX
#define UNE_DO_PARSE
#define UNE_DO_INTERPRET

#ifdef UNE_DEBUG
  // #define UNE_DISPLAY_BAR
  // #define UNE_DISPLAY_TOKENS
  // #define UNE_DISPLAY_NODES
  #define UNE_DISPLAY_RESULT
  #define UNE_DISPLAY_MEMORY_REPORT

  #define UNE_DEBUG_MALLOC_COUNTER

  // #define UNE_DEBUG_LOG_INTERPRET
  // #define UNE_DEBUG_LOG_FREE

  extern int err;
#endif

#include <stdlib.h>
#include <wchar.h>
#include <stdint.h>
#include <stdbool.h>

// IO Number Types
typedef int64_t une_int;
typedef double une_flt;

// TCC implements a nonportable swprintf.
#if defined(__TINYC__)
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

// Temporary Internal Error Response and Logging Tools
#define WERR(msg, ...)\
{\
  wprintf(\
    UNE_COLOR_FAIL L"%hs:%d: " msg "\33[0m\n",\
    __FILE__, __LINE__, ##__VA_ARGS__\
  );\
  exit(1);\
}
#define LOG(msg, ...)\
{\
  wprintf(\
    UNE_COLOR_NEUTRAL L"\33[7m%hs:%d: " msg "\33[0m\n",\
    __FILE__, __LINE__, ##__VA_ARGS__\
  );\
}
#define LOGD(num)\
{\
  wprintf(\
    UNE_COLOR_NEUTRAL L"\33[7m%hs:%d: %lld\33[0m\n",\
    __FILE__, __LINE__, num\
  );\
}
#define LOGC(ch)\
{\
  wprintf(\
    UNE_COLOR_NEUTRAL L"\33[7m%hs:%d: '%c'\33[0m\n",\
    __FILE__, __LINE__, ch\
  );\
}
#define LOGS(str)\
{\
  wprintf(\
    UNE_COLOR_NEUTRAL L"\33[7m%hs:%d: \"%ls\"\33[0m\n",\
    __FILE__, __LINE__, str\
  );\
}

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
