:: Make Une
@echo off & setlocal enableDelayedExpansion

::#region Preferences

:: 1, 0
set clear=1
set release=0
set debug=1
set debug_gdb=1

:: gcc, clang
set compiler=gcc

::#endregion Preferences

::#region Options

set O=
if %release% Equ 1 (
  if %compiler% Equ gcc (
    set "O=-O3 "
  ) else if %compiler% Equ clang (
    set "O=-O3 "
  )
)

:: -Wextra (-Wshadow?)
set "flags=-pedantic -Wall -Wno-switch "
if %compiler% Equ clang (
  set "flags=!flags!-Wno-deprecated "
)
if %debug_gdb% Equ 1 (
  set "flags=!flags!-g3 "
)
if %debug% Equ 1 (
  set "flags=!flags!-DUNE_DEBUG "
) else (
  set "flags=!flags!-Wno-unused-function "
)

::#endregion Options

set src=^
main.c ^
une.c ^
interpreter.c ^
parser.c ^
lexer.c ^
types/context.c ^
types/symbols.c ^
types/error.c ^
types/result.c ^
types/node.c ^
types/token.c ^
types/interpreter_state.c ^
types/parser_state.c ^
types/lexer_state.c ^
tools.c ^
stream.c ^
builtin.c ^
util/memdbg.c

pushd src
if %clear% Equ 1 (
  cls
)
%compiler% %O%%flags%%src% -o ../une.exe
set el=%errorlevel%
popd
if %el% Equ 0 (
  if "%*" nEq "" (
    une %*
  )
)

echo [35m%compiler% %O%%flags%[0m

exit /b %el%
