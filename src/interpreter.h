/*
interpreter.h - Une
Modified 2023-02-11
*/

#ifndef UNE_INTERPRETER_H
#define UNE_INTERPRETER_H

/* Header-specific includes. */
#include "primitive.h"
#include "types/interpreter_state.h"
#include "types/error.h"
#include "types/node.h"
#include "types/result.h"
#include "types/symbols.h"
#include "builtin_functions.h"

/*
*** Interface.
*/

/*
Interpreter function template.
*/
#define une_interpreter__(id__, ...)\
  une_static__ une_result (id__)(une_error *error, une_interpreter_state *is, une_node *node, ##__VA_ARGS__)

une_result une_interpret(une_error *error, une_interpreter_state *is, une_node *node);

une_interpreter__(une_interpret_as, une_result_type type);

une_interpreter__(une_interpret_void);
une_interpreter__(une_interpret_int);
une_interpreter__(une_interpret_flt);
une_interpreter__(une_interpret_str);
une_interpreter__(une_interpret_list);
une_interpreter__(une_interpret_object);
une_interpreter__(une_interpret_function);
une_interpreter__(une_interpret_builtin);
une_interpreter__(une_interpret_stmts);
une_interpreter__(une_interpret_cop);
une_interpreter__(une_interpret_not);
une_interpreter__(une_interpret_and);
une_interpreter__(une_interpret_or);
une_interpreter__(une_interpret_nullish);
une_interpreter__(une_interpret_equ);
une_interpreter__(une_interpret_neq);
une_interpreter__(une_interpret_gtr);
une_interpreter__(une_interpret_geq);
une_interpreter__(une_interpret_lss);
une_interpreter__(une_interpret_leq);
une_interpreter__(une_interpret_add);
une_interpreter__(une_interpret_sub);
une_interpreter__(une_interpret_mul);
une_interpreter__(une_interpret_div);
une_interpreter__(une_interpret_fdiv);
une_interpreter__(une_interpret_mod);
une_interpreter__(une_interpret_pow);
une_interpreter__(une_interpret_neg);
une_interpreter__(une_interpret_seek, bool existing_only);
une_interpreter__(une_interpret_idx_seek);
une_interpreter__(une_interpret_member_seek);
une_interpreter__(une_interpret_assign);
une_interpreter__(une_interpret_get);
une_interpreter__(une_interpret_idx_get);
une_interpreter__(une_interpret_member_get);
une_interpreter__(une_interpret_call);
une_interpreter__(une_interpret_for_range);
une_interpreter__(une_interpret_for_element);
une_interpreter__(une_interpret_while);
une_interpreter__(une_interpret_if);
une_interpreter__(une_interpret_continue);
une_interpreter__(une_interpret_break);
une_interpreter__(une_interpret_return);
une_interpreter__(une_interpret_exit);
une_interpreter__(une_interpret_cover);
une_interpreter__(une_interpret_concatenate);

#endif /* !UNE_INTERPRETER_H */
