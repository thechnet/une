#!/bin/bash
# Make Une

#region Preferences

# 1, 0
# FIXME:
debug=1

# clang
compiler="clang"

#endregion Preferences

#region Defines

if [ $debug==1 ]; then
  UNE_DEBUG=1
fi

#endregion Defines

src=\
"une.c "\
"main.c "\
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
"builtin.c"\

>/dev/null pushd src
$compiler -Wno-switch $src -o ../une
el=$?
>/dev/null popd

exit $el