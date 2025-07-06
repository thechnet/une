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
list(APPEND options "USES_UCRT|Enable when using UCRT on Windows|OFF")
list(APPEND options "DBG_DISPLAY_EXTENDED_ERROR|Show extended error information|ON")
list(APPEND options "DBG_SIZES|Default most sizes to 1|ON")
list(APPEND options "DBG_REPORT|Enable communication with test.py|ON")
list(APPEND options "DBG_MEMDBG|Use memdbg|ON")
list(APPEND options "DBG_DISPLAY_TOKENS|Display processed tokens|ON")
list(APPEND options "DBG_DISPLAY_NODES|Display processed nodes|ON")
list(APPEND options "DBG_DISPLAY_RESULT|Display processed result|ON")
list(APPEND options "DBG_LOG_PARSE|Log parse calls|OFF")
list(APPEND options "DBG_LOG_INTERPRET|Log interpretation calls|OFF")
list(APPEND options "DBG_NO_LEX|Skip lexing|OFF")
list(APPEND options "DBG_NO_PARSE|Skip parsing|OFF")
list(APPEND options "DBG_NO_INTERPRET|Skip interpretation|OFF")
list(APPEND options "DBG_FSANITIZE|Use -fsanitize flags|OFF")
list(APPEND options "DBG_COVERAGE|Enable instrumentation|OFF")
list(APPEND options "DBG_USE_ABSOLUTE_MODULE_PATHS|Use absolute module paths|OFF")
foreach(option IN LISTS options)
	string(REPLACE "|" ";" option_list ${option})
	list(GET option_list 0 id)
	list(GET option_list 1 desc)
	list(GET option_list 2 val)
	option(UNE_${id} "${desc}." ${val})
endforeach()

# Executable.
file(GLOB_RECURSE une_source "src/*.c")
add_executable(une ${une_source})

# Libraries.
target_link_libraries(une m)
if(WIN32)
	target_link_libraries(une winmm)
endif()

# Include directories.
target_include_directories(une PUBLIC "${PROJECT_BINARY_DIR}")

# 32-bit
if(UNE_32BIT)
	set_target_properties(une PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")
endif()

# Standard flags.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(une PRIVATE -Wall -Wextra -Wno-unused-value -Wno-implicit-fallthrough)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
	target_compile_options(une PRIVATE -Weverything)
	if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.0)
		target_compile_options(une PRIVATE -Wno-unsafe-buffer-usage -Wno-poison-system-directories)
	endif()
endif()
target_compile_options(une PRIVATE
	-pedantic
	-Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments -Wno-declaration-after-statement -Wno-used-but-marked-unused
	-Wno-padded -Wno-disabled-macro-expansion -Wno-missing-prototypes -Wno-missing-variable-declarations -Wno-unused-macros
)
if(UNE_USES_UCRT)
	add_definitions(-D__USE_MINGW_ANSI_STDIO=1) # See https://github.com/mstorsjo/llvm-mingw/issues/373.
endif()

# Build types.
if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_definitions(-DUNE_DEBUG)
	target_compile_options(une PRIVATE
		-g3 -O0 -Wno-switch -Wno-switch-enum
	)
	if(UNE_DBG_MEMDBG)
		add_definitions(-DMEMDBG_ENABLE)
		target_sources(une PRIVATE "src/deprecated/memdbg.c")
		target_compile_options(une PRIVATE
			-Wno-unused-macros
		)
	endif()
	if(UNE_DBG_FSANITIZE)
		target_compile_options(une PRIVATE
			-fsanitize=address,signed-integer-overflow -fsanitize-address-use-after-return=always -fsanitize-address-use-after-scope -fno-omit-frame-pointer -O1 -fno-optimize-sibling-calls 
		)
		target_link_options(une PRIVATE
			-fsanitize=address,signed-integer-overflow
		)
	endif()
	if(UNE_DBG_COVERAGE)
		target_compile_options(une PRIVATE
			-fprofile-arcs -ftest-coverage
		)
		target_link_options(une PRIVATE
			-fprofile-arcs -ftest-coverage
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
