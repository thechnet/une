# Project.
cmake_minimum_required(VERSION 3.10)
project(une
  VERSION 0.10.0
  DESCRIPTION "The Une programming language"
  LANGUAGES C
)

# Default build type.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified, using Release.")
endif()

# Options.
list(APPEND options "DEBUG_DISPLAY_EXTENDED_ERROR|Show extended error information|ON")
list(APPEND options "DEBUG_WATCHDOG|Use Watchdog|ON")
list(APPEND options "DEBUG_SIZES|Default most sizes to 1|ON")
list(APPEND options "DEBUG_REPORT|Enable communication with test.py|ON")
list(APPEND options "DEBUG_MEMDBG|Use memdbg|OFF")
list(APPEND options "DISPLAY_TOKENS|Display processed tokens|OFF")
list(APPEND options "DISPLAY_NODES|Display processed nodes|OFF")
list(APPEND options "DISPLAY_RESULT|Display processed result|ON")
list(APPEND options "DEBUG_LOG_PARSE|Log parse calls|OFF")
list(APPEND options "DEBUG_LOG_INTERPRET|Log interpretation calls|OFF")
list(APPEND options "NO_LEX|Skip lexing|OFF")
list(APPEND options "NO_PARSE|Skip parsing|OFF")
list(APPEND options "NO_INTERPRET|Skip interpretation|OFF")
foreach(option IN LISTS options)
  string(REPLACE "|" ";" option_list ${option})
  list(GET option_list 0 id)
  list(GET option_list 1 desc)
  list(GET option_list 2 val)
  option(UNE_${id} "Debug: ${desc}." ${val})
endforeach()

# Source.
add_executable(une
  # Backend.
  src/une.c
  src/interpreter.c
  src/parser.c
  src/lexer.c
  src/types/context.c
  src/types/symbols.c
  src/types/error.c
  src/types/result.c
  src/types/node.c
  src/types/token.c
  src/types/interpreter_state.c
  src/types/parser_state.c
  src/types/lexer_state.c
  src/datatypes/datatypes.c
  src/datatypes/int.c
  src/datatypes/flt.c
  src/datatypes/str.c
  src/datatypes/list.c
  src/datatypes/void.c
  src/datatypes/function.c
  src/datatypes/builtin.c
  src/tools.c
  src/stream.c
  src/builtin_functions.c
  # Frontend.
  src/main.c
)

# Include directories.
target_include_directories(une PUBLIC
  "${PROJECT_BINARY_DIR}"
)

# Standard flags.
target_compile_options(une PRIVATE
  -pedantic -Weverything -Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments -Wno-declaration-after-statement -Wno-used-but-marked-unused -Wno-padded -Wno-disabled-macro-expansion -Wno-missing-prototypes -Wno-missing-variable-declarations
)

# Build types.
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions(-DUNE_DEBUG)
  target_compile_options(une PRIVATE
    -g3 -Wno-switch -Wno-switch-enum
  )
  if(UNE_DEBUG_WATCHDOG)
    add_definitions(-DWATCHDOG)
    target_link_libraries(une "${PROJECT_SOURCE_DIR}/watchdog/build/libwatchdog.a")
    target_compile_options(une PRIVATE
      -Wno-keyword-macro -Werror=embedded-directive
    )
  endif()
  if(UNE_DEBUG_MEMDBG)
    add_definitions(-DMEMDBG_ENABLE)
    target_sources(une PRIVATE "src/util/memdbg.c")
    target_compile_options(une PRIVATE
      -Wno-unused-macros
    )
  endif()
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
  target_compile_options(une PRIVATE
    -O3
    -Wno-unused-function -Wno-switch -Wno-switch-enum
  )
else()
  message(FATAL_ERROR "Illegal build type '${CMAKE_BUILD_TYPE}'.")
endif()

foreach(option IN LISTS options)
  string(REPLACE "|" ";" option_list ${option})
  list(GET option_list 0 id)
  list(GET option_list 2 val)
  if(${val} STREQUAL ON)
    add_definitions(-DUNE_${id})
  endif()
endforeach()

configure_file(src/cmake.h.in cmake.h)
