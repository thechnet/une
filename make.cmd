:: Make Une
@echo off
setlocal enableDelayedExpansion

::#region Preferences

:: 1, 0
set release=0
:: FIXME:
set debug=1

:: gcc, clang, tcc, vs
set compiler=gcc

::#endregion Preferences

::#region Options

if %debug% Equ 1 (
  set UNE_DEBUG=1
)

set O=
if %release% Equ 1 (
  if %compiler% Equ gcc (
    set O=-O3
  ) else if %compiler% Equ clang (
    set O=-O3
  )
)

set flags=
if %compiler% Equ clang (
  set flags=-Wno-deprecated
)

::#endregion Options

set src=^
main.c ^
interpreter.c ^
parser.c ^
lexer.c ^
types/context.c ^
types/symbols.c ^
types/error.c ^
types/result.c ^
types/node.c ^
types/token.c ^
tools.c ^
builtin.c

pushd src
%compiler% %O% %flags% %src% -o ../une.exe
set el=%errorlevel%
popd

exit /b %el%