#!/bin/bash
# Make Une

#region Preferences

# 1, 0
clear=1
release=0
debug=1
debug_gdb=1

# gcc, clang
compiler="gcc"

#endregion Preferences

#region Options

unset O
if [ $release==1 ]; then
  if [ $compiler=="gcc" ]; then
    O="-O3"
  elif [ $compiler=="clang" ]; then
    O="-O3"
  fi
fi

flags="-Wall"
if [ $compiler=="clang" ]; then
  flags="${flags} -Wno-deprecated -Wno-switch"
fi
if [ $debug_gdb==1 ]; then
  flags="${flags} -g -Wno-switch"
fi
# if [ $debug==1 ]; then
#   flags="${flags} -DUNE_DEBUG"
# fi

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
"builtin.c"

>/dev/null pushd src
if [ $clear==1 ]; then
  clear
fi
$compiler $O $flags $src -o ../une
el=$?
>/dev/null popd
if [ "$*"!="" ]; then
  if [ $el==0 ]; then
    ./une $*
  fi
fi

echo "Done."

exit $el
