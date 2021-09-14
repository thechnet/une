/*
interpreter.h - Une
Modified 2021-09-14
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
#define __une_interpreter(__id, ...)\
  __une_static une_result (__id)(une_error *error, une_interpreter_state *is, une_node *node, ##__VA_ARGS__)

une_result une_interpret(une_error *error, une_interpreter_state *is, une_node *node);

__une_interpreter(une_interpret_as, une_result_type type);

__une_interpreter(une_interpret_int);
__une_interpreter(une_interpret_flt);
__une_interpreter(une_interpret_str);
__une_interpreter(une_interpret_list);
__une_interpreter(une_interpret_function);
__une_interpreter(une_interpret_builtin);
__une_interpreter(une_interpret_stmts);
__une_interpreter(une_interpret_cop);
__une_interpreter(une_interpret_not);
__une_interpreter(une_interpret_and);
__une_interpreter(une_interpret_or);
__une_interpreter(une_interpret_equ);
__une_interpreter(une_interpret_neq);
__une_interpreter(une_interpret_gtr);
__une_interpreter(une_interpret_geq);
__une_interpreter(une_interpret_lss);
__une_interpreter(une_interpret_leq);
__une_interpreter(une_interpret_add);
__une_interpreter(une_interpret_sub);
__une_interpreter(une_interpret_mul);
__une_interpreter(une_interpret_div);
__une_interpreter(une_interpret_fdiv);
__une_interpreter(une_interpret_mod);
__une_interpreter(une_interpret_pow);
__une_interpreter(une_interpret_neg);
__une_interpreter(une_interpret_set);
__une_interpreter(une_interpret_set_idx);
__une_interpreter(une_interpret_get);
__une_interpreter(une_interpret_get_idx);
__une_interpreter(une_interpret_call);
__une_interpreter(une_interpret_for);
__une_interpreter(une_interpret_while);
__une_interpreter(une_interpret_if);
__une_interpreter(une_interpret_continue);
__une_interpreter(une_interpret_break);
__une_interpreter(une_interpret_return);

#endif /* !UNE_INTERPRETER_H */
