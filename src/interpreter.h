/*
interpreter.h - Une
Modified 2023-12-10
*/

#ifndef UNE_INTERPRETER_H
#define UNE_INTERPRETER_H

/* Header-specific includes. */
#include "common.h"
#include "struct/interpreter_state.h"
#include "struct/error.h"
#include "struct/node.h"
#include "struct/result.h"
#include "struct/association.h"
#include "natives.h"
#include "struct/engine.h"

/*
Interpreter function template.
*/
#define une_interpreter__(name__, ...)\
	une_static__ une_result (name__)(une_node *node, ##__VA_ARGS__)

/*
*** Interface.
*/

une_result une_interpret(une_node *node);

/*
*** Interpreter table.
*/

une_interpreter__(une_interpret_void);
une_interpreter__(une_interpret_int);
une_interpreter__(une_interpret_flt);
une_interpreter__(une_interpret_str);
une_interpreter__(une_interpret_list);
une_interpreter__(une_interpret_object);
une_interpreter__(une_interpret_function);
une_interpreter__(une_interpret_native);
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
une_interpreter__(une_interpret_seek);
une_interpreter__(une_interpret_idx_seek);
une_interpreter__(une_interpret_member_seek);
une_interpreter__(une_interpret_assign);
une_interpreter__(une_interpret_assign_add);
une_interpreter__(une_interpret_assign_sub);
une_interpreter__(une_interpret_assign_pow);
une_interpreter__(une_interpret_assign_mul);
une_interpreter__(une_interpret_assign_fdiv);
une_interpreter__(une_interpret_assign_div);
une_interpreter__(une_interpret_assign_mod);
une_interpreter__(une_interpret_call);
une_interpreter__(une_interpret_for_range);
une_interpreter__(une_interpret_for_element);
une_interpreter__(une_interpret_while);
une_interpreter__(une_interpret_if);
une_interpreter__(une_interpret_assert);
une_interpreter__(une_interpret_continue);
une_interpreter__(une_interpret_break);
une_interpreter__(une_interpret_return);
une_interpreter__(une_interpret_exit);
une_interpreter__(une_interpret_any);
une_interpreter__(une_interpret_all);
une_interpreter__(une_interpret_cover);
une_interpreter__(une_interpret_concatenate);
une_interpreter__(une_interpret_this);

/*
*** Helpers.
*/

une_interpreter__(une_interpret_as, une_result_kind kind);
une_interpreter__(une_interpret_seek_or_create, bool existing_only);
une_interpreter__(une_interpret_idx_seek_index);
une_interpreter__(une_interpret_idx_seek_range);
une_interpreter__(une_interpret_comparison, une_int (*comparator)(une_result, une_result));

#endif /* !UNE_INTERPRETER_H */
