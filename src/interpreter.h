/*
interpreter.h - Une
Updated 2021-05-22
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
#include "types/instance.h"

// Public Interpreter Interface.
une_result une_interpret(une_instance *inst, une_node *node);

// Node Interpreters.
static une_result une_interpret_stmts  (une_instance *inst, une_node *node);
static une_result une_interpret_add    (une_instance *inst, une_node *node);
static une_result une_interpret_sub    (une_instance *inst, une_node *node);
static une_result une_interpret_mul    (une_instance *inst, une_node *node);
static une_result une_interpret_div    (une_instance *inst, une_node *node);
static une_result une_interpret_fdiv   (une_instance *inst, une_node *node);
static une_result une_interpret_mod    (une_instance *inst, une_node *node);
static une_result une_interpret_neg    (une_instance *inst, une_node *node);
static une_result une_interpret_pow    (une_instance *inst, une_node *node);
static une_result une_interpret_not    (une_instance *inst, une_node *node);
static une_result une_interpret_equ    (une_instance *inst, une_node *node);
static une_result une_interpret_neq    (une_instance *inst, une_node *node);
static une_result une_interpret_gtr    (une_instance *inst, une_node *node);
static une_result une_interpret_geq    (une_instance *inst, une_node *node);
static une_result une_interpret_lss    (une_instance *inst, une_node *node);
static une_result une_interpret_leq    (une_instance *inst, une_node *node);
static une_result une_interpret_and    (une_instance *inst, une_node *node);
static une_result une_interpret_or     (une_instance *inst, une_node *node);
static une_result une_interpret_cop    (une_instance *inst, une_node *node);
static une_result une_interpret_get_idx(une_instance *inst, une_node *node);
static une_result une_interpret_get    (une_instance *inst, une_node *node);
static une_result une_interpret_set_idx(une_instance *inst, une_node *node);
static une_result une_interpret_set    (une_instance *inst, une_node *node);
static une_result une_interpret_for    (une_instance *inst, une_node *node);
static une_result une_interpret_while  (une_instance *inst, une_node *node);
static une_result une_interpret_if     (une_instance *inst, une_node *node);
static une_result une_interpret_def    (une_instance *inst, une_node *node);
static une_result une_interpret_call   (une_instance *inst, une_node *node);
static une_result une_interpret_list   (une_instance *inst, une_node *node);

// Expect specific result type.
static une_result une_interpret_as(
  une_instance *inst, une_node *node, une_result_type type
);

// Function Calling.
static une_result une_interpret_call_def(
  une_instance *inst, une_function *fn, une_result *args
);
static une_result une_interpret_call_builtin(
  une_instance *inst, une_builtin_type type, une_result *args, une_position pos
);

// GET_IDX Helpers.
static une_result une_interpret_get_idx_list(une_result list, une_int index);
static une_result une_interpret_get_idx_str (une_result str,  une_int index);

#endif /* !UNE_INTERPRETER_H */
