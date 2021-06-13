:: Make Une
@echo off & setlocal enableDelayedExpansion

::#region Preferences

:: 1, 0
set clear=1
set release=0
set debug=1
set debug_gdb=1

:: gcc, clang, tcc
set compiler=gcc

::#endregion Preferences

::#region Options

set O=
if %release% Equ 1 (
  if %compiler% Equ gcc (
    set O=-O3
  ) else if %compiler% Equ clang (
    set O=-O3
  )
)
if %debug_gdb% Equ 1 (
  set O=-Og
)

set flags=
if %compiler% Equ clang (
  set "flags=!flags! -Wno-deprecated"
)
if %debug_gdb% Equ 1 (
  set "flags=!flags! -g"
)
@REM if %debug% Equ 1 (
@REM   set "flags=!flags! -DUNE_DEBUG"
@REM )

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
builtin.c

pushd src
if %clear% Equ 1 (
  cls
)
%compiler% %O% %flags% %src% -o ../une.exe
set el=%errorlevel%
popd
if %el% Equ 0 (
  une %*
)

echo Done.

exit /b %el%
