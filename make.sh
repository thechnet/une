#!/bin/bash
# Make Une

#region ***************************** Preferences

# Allowed: 1, 0
clear=1
release=0
debug=1
debug_gdb=0

# Allowed: gcc, clang
compiler="clang"

out="une"

#endregion ***************************** Preferences

# File Extension
if [ "$OSTYPE" == "msys" ]; then
  out="${out}.exe"
fi

# Optimization Level
unset O
if [ $release -eq 1 ]; then
  O="-O3 "
fi

# Warnings
flags="-pedantic -Wall " # -Wextra (-Wshadow?)
if [ "$compiler" == "clang" ]; then
  flags="${flags}-Wno-deprecated -Wno-switch -Wno-gnu-zero-variadic-macro-arguments "
fi

# Debug Mode
if [ $debug_gdb -eq 1 ]; then
  flags="${flags}-g3 "
fi
if [ $debug -eq 1 ]; then
  flags="${flags}-DUNE_DEBUG "
else
  flags="${flags}-Wno-unused-function "
fi

# Source Files
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
"datatypes/datatypes.c "\
"datatypes/int.c "\
"datatypes/flt.c "\
"datatypes/str.c "\
"datatypes/list.c "\
"datatypes/void.c "\
"datatypes/function.c "\
"datatypes/builtin.c "\
"tools.c "\
"stream.c "\
"builtin_functions.c"
if [ $debug -eq 1 ]; then
  src+=" util/memdbg.c"
fi

# Compilation
>/dev/null pushd src
if [ $clear -eq 1 ]; then
  clear
fi
$compiler $O$flags$src -o ../$out
el=$?
>/dev/null popd

# Une Arguments
if [ $el -eq 0 ]; then
  if [ -n "$*" ]; then
    ./$out $*
    el=$?
  fi
fi

# Feedback
echo "$compiler: $out [$el] ($O$flags)"
exit $el
