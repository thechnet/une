# Version.
file(READ VERSION.txt FILE_VERSION)
string(REGEX REPLACE "\n$" "" VERSION_STRING "${FILE_VERSION}")

# Project.
cmake_minimum_required(VERSION 3.10)
project(une
  VERSION ${VERSION_STRING}
  DESCRIPTION "The Une programming language"
  LANGUAGES C
)

# Default build type.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified, using Release.")
endif()

# Options.
list(APPEND options "32BIT|Build for 32-bit machines|OFF")
list(APPEND options "DEBUG_DISPLAY_EXTENDED_ERROR|Debug: Show extended error information|ON")
list(APPEND options "DEBUG_WATCHDOG|Debug: Use Watchdog|OFF")
list(APPEND options "DEBUG_SIZES|Debug: Default most sizes to 1|ON")
list(APPEND options "DEBUG_REPORT|Debug: Enable communication with test.py|ON")
list(APPEND options "DEBUG_MEMDBG|Debug: Use memdbg|ON")
list(APPEND options "DISPLAY_TOKENS|Debug: Display processed tokens|ON")
list(APPEND options "DISPLAY_NODES|Debug: Display processed nodes|ON")
list(APPEND options "DISPLAY_RESULT|Debug: Display processed result|ON")
list(APPEND options "DEBUG_LOG_PARSE|Debug: Log parse calls|OFF")
list(APPEND options "DEBUG_LOG_INTERPRET|Debug: Log interpretation calls|OFF")
list(APPEND options "NO_LEX|Debug: Skip lexing|OFF")
list(APPEND options "NO_PARSE|Debug: Skip parsing|OFF")
list(APPEND options "NO_INTERPRET|Debug: Skip interpretation|OFF")
foreach(option IN LISTS options)
  string(REPLACE "|" ";" option_list ${option})
  list(GET option_list 0 id)
  list(GET option_list 1 desc)
  list(GET option_list 2 val)
  option(UNE_${id} "${desc}." ${val})
endforeach()

# Source.
add_executable(une
  # Backend.
  src/une.c
  src/interpreter.c
  src/parser.c
  src/lexer.c
  src/types/context.c
  src/types/association.c
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
  src/datatypes/object.c
  src/datatypes/void.c
  src/datatypes/function.c
  src/datatypes/builtin.c
  src/tools.c
  src/stream.c
  src/builtin_functions.c
  # Frontend.
  src/main.c
)

# libm
target_link_libraries(une m)

# Include directories.
target_include_directories(une PUBLIC
  "${PROJECT_BINARY_DIR}"
)

# 32-bit
if(UNE_32BIT)
  set_target_properties(une PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")
endif()

# Standard flags.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  target_compile_options(une PRIVATE -Wall -Wextra -Wno-unused-value -Wno-implicit-fallthrough)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(une PRIVATE -Weverything)
endif()
target_compile_options(une PRIVATE
  -pedantic -Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments -Wno-declaration-after-statement -Wno-used-but-marked-unused -Wno-padded -Wno-disabled-macro-expansion -Wno-missing-prototypes -Wno-missing-variable-declarations -Wno-unsafe-buffer-usage
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
    target_compile_options(une PRIVATE -Wno-keyword-macro)
    if(CMAKE_C_COMPILE_ID MATCHES "Clang")
      target_compile_options(une PRIVATE -Werror=embedded-directive)
    endif()
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

# Preprocessor definitions.
foreach(option IN LISTS options)
  string(REPLACE "|" ";" option_list ${option})
  list(GET option_list 0 id)
  list(GET option_list 2 val)
  if("${UNE_${id}}" STREQUAL "ON")
    add_definitions(-DUNE_${id})
  endif()
endforeach()

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE VERSION_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(src/cmake.h.in cmake.h)
