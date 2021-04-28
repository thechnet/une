/*
interpreter.h - Une
Updated 2021-04-28
*/

#ifndef UNE_INTERPRETER_H
#define UNE_INTERPRETER_H

#include "primitive.h"
#include "types/error.h"
#include "types/node.h"
#include "types/result.h"
#include "types/symbols.h"
#include "types/context.h"
#include "tools.h"
#include <string.h>
#include <math.h>

une_result une_interpret(une_node *node, une_context *context);
une_result une_interpret_stmts(une_node *node, une_context *context);
une_result une_interpret_add(une_node *node, une_context *context);
une_result une_interpret_sub(une_node *node, une_context *context);
une_result une_interpret_mul(une_node *node, une_context *context);
une_result une_interpret_div(une_node *node, une_context *context);
une_result une_interpret_fdiv(une_node *node, une_context *context);
une_result une_interpret_mod(une_node *node, une_context *context);
une_result une_interpret_neg(une_node *node, une_context *context);
une_result une_interpret_pow(une_node *node, une_context *context);
une_result une_interpret_not(une_node *node, une_context *context);
une_result une_interpret_equ(une_node *node, une_context *context);
une_result une_interpret_neq(une_node *node, une_context *context);
une_result une_interpret_gtr(une_node *node, une_context *context);
une_result une_interpret_geq(une_node *node, une_context *context);
une_result une_interpret_lss(une_node *node, une_context *context);
une_result une_interpret_leq(une_node *node, une_context *context);
une_result une_interpret_and(une_node *node, une_context *context);
une_result une_interpret_or(une_node *node, une_context *context);
une_result une_interpret_cop(une_node *node, une_context *context);
une_result une_interpret_idx_get(une_node *node, une_context *context);
une_result une_interpret_set(une_node *node, une_context *context);
une_result une_interpret_set_idx(une_node *node, une_context *context);
une_result une_interpret_get(une_node *node, une_context *context);
une_result une_interpret_for(une_node *node, une_context *context);
une_result une_interpret_while(une_node *node, une_context *context);
une_result une_interpret_if(une_node *node, une_context *context);
une_result une_interpret_def(une_node *node, une_context *context);
une_result une_interpret_call(une_node *node, une_context *context);
une_result une_interpret_list(une_node *node, une_context *context);

#endif /* !UNE_INTERPRETER_H */
