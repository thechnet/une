/*
interpreter.h - Une
Updated 2021-04-29
*/

#ifndef UNE_INTERPRETER_H
#define UNE_INTERPRETER_H

#include <string.h>
#include <math.h>
#include "primitive.h"
#include "tools.h"
#include "builtin.h"
#include "types/error.h"
#include "types/node.h"
#include "types/result.h"
#include "types/symbols.h"
#include "types/context.h"

une_result une_interpret(une_node *node, une_context *context);

static une_result une_interpret_as(une_result_type type, une_node *node, une_context *context);

static une_result une_interpret_stmts(une_node *node, une_context *context);
static une_result une_interpret_add(une_node *node, une_context *context);
static une_result une_interpret_sub(une_node *node, une_context *context);
static une_result une_interpret_mul(une_node *node, une_context *context);
static une_result une_interpret_div(une_node *node, une_context *context);
static une_result une_interpret_fdiv(une_node *node, une_context *context);
static une_result une_interpret_mod(une_node *node, une_context *context);
static une_result une_interpret_neg(une_node *node, une_context *context);
static une_result une_interpret_pow(une_node *node, une_context *context);
static une_result une_interpret_not(une_node *node, une_context *context);
static une_result une_interpret_equ(une_node *node, une_context *context);
static une_result une_interpret_neq(une_node *node, une_context *context);
static une_result une_interpret_gtr(une_node *node, une_context *context);
static une_result une_interpret_geq(une_node *node, une_context *context);
static une_result une_interpret_lss(une_node *node, une_context *context);
static une_result une_interpret_leq(une_node *node, une_context *context);
static une_result une_interpret_and(une_node *node, une_context *context);
static une_result une_interpret_or(une_node *node, une_context *context);
static une_result une_interpret_cop(une_node *node, une_context *context);
static une_result une_interpret_get_idx(une_node *node, une_context *context);
static une_result une_interpret_get_idx_list(une_result list, une_int index);
static une_result une_interpret_get_idx_str(une_result str, une_int index);
static une_result une_interpret_get(une_node *node, une_context *context);
static une_result une_interpret_set_idx(une_node *node, une_context *context);
static une_result une_interpret_set(une_node *node, une_context *context);
static une_result une_interpret_for(une_node *node, une_context *context);
static une_result une_interpret_while(une_node *node, une_context *context);
static une_result une_interpret_if(une_node *node, une_context *context);
static une_result une_interpret_def(une_node *node, une_context *context);
static une_result une_interpret_call_def(une_function *fn, une_result *args, une_context *context);
static une_result une_interpret_call_builtin(une_builtin_type type, une_result *args, une_context *context, une_position pos);
static une_result une_interpret_call(une_node *node, une_context *context);
static une_result une_interpret_list(une_node *node, une_context *context);

#endif /* !UNE_INTERPRETER_H */
