#!/bin/bash
# Make Une

#region Preferences

# 1, 0
clear=1
release=0
debug=0
debug_gdb=0

# gcc, clang
compiler="gcc"

#endregion Preferences

#region Options

unset O
if [ $release -eq 1 ]; then
  if [ $compiler=="gcc" ]; then
    O="-O3 "
  elif [ $compiler=="clang" ]; then
    O="-O3 "
  fi
fi

# -Wextra (-Wshadow?)
flags="-pedantic -Wall -Wno-switch "
if [ $compiler=="clang" ]; then
  flags="${flags}-Wno-deprecated "
fi
if [ $debug_gdb -eq 1 ]; then
  flags="${flags}-g3 "
fi
if [ $debug -eq 1 ]; then
  flags="${flags}-DUNE_DEBUG "
else
  flags="${flags}-Wno-unused-function "
fi

#endregion Options

src=\
"main.c "\
"une.c "\
"interpreter.c "\
"parser.c "\
"lexer.c "\
"types/context.c "\
"types/symbols.c "\
"types/error.c "\
"types/result.c "\
"types/node.c "\
"types/token.c "\
"types/interpreter_state.c "\
"types/parser_state.c "\
"types/lexer_state.c "\
"tools.c "\
"stream.c "\
"builtin.c "\
"util/memdbg.c"

>/dev/null pushd src
if [ $clear==1 ]; then
  clear
fi
$compiler $O$flags$src -o ../une
el=$?
>/dev/null popd
if [ $el -eq 0 ]; then
  if [ -n "$*" ]; then
    ./une $*
    el=$?
  fi
fi

echo $'\e[35m'"$compiler $O$flags"$'\e[0m'

exit $el
