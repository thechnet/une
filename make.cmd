:: Make Une
@echo off
setlocal enableDelayedExpansion

::#region Preferences

:: 1, 0
:: FIXME:
set debug=1

:: gcc, clang, tcc, vs
set compiler=gcc

::#endregion Preferences

::#region Defines

if %debug% Equ 1 (
  set UNE_DEBUG=1
)

::#endregion Defines

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
tools.c

pushd src
%compiler% %src% -o ../une.exe
popd

exit /b