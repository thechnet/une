/*
une.h - Une
Updated 2021-04-17
*/

#ifndef UNE_UNE_H
#define UNE_UNE_H

// OS and compiler-specific differences
#if defined(__TINYC__)
  #include <io.h>
  #define UNE_USE_NONPORTABLE_SWPRINTF
#elif defined(__clang__)
  #ifdef __APPLE__
    #include <unistd.h>
    #include <sys/uio.h>
  #else
    #include <io.h>
    #define access _access
    #define R_OK 4
  #endif
#elif defined(_MSC_VER)
  #include <io.h>
  #define R_OK 4
#else
  #include <unistd.h>
#endif

#include "primitive.h"
#include "tools.h"
#include "types/symbols.h"
#include "types/error.h"
#include "types/token.h"
#include "types/node.h"
#include "types/result.h"
#include "types/context.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

#endif /* !UNE_UNE_H */