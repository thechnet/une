/*
interpreter.h - Une
Updated 2021-06-04
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
#include "types/interpreter_state.h"

// typedef une_result __interpreter(
//   une_error *error,
//   une_interpreter_state *is,
//   une_node *node
// );

// Public Interpreter Interface.
une_result une_interpret(
  une_error *error,
  une_interpreter_state *is,
  une_node *node
);

// Node Interpreters.
static une_result une_interpret_stmts  (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_add    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_sub    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_mul    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_div    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_fdiv   (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_mod    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_neg    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_pow    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_not    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_equ    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_neq    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_gtr    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_geq    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_lss    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_leq    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_and    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_or     (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_cop    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_get_idx(une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_get    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_set_idx(une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_set    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_for    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_while  (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_if     (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_def    (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_call   (une_error *error, une_interpreter_state *is, une_node *node);
static une_result une_interpret_list   (une_error *error, une_interpreter_state *is, une_node *node);

// Expect specific result type.
static une_result une_interpret_as(
  une_error *error,
  une_interpreter_state *is,
  une_node *node,
  une_result_type type
);

// Function Calling.
static une_result une_interpret_call_def(
  une_error *error,
  une_interpreter_state *is,
  une_function *fn,
  une_result *args
);
static une_result une_interpret_call_builtin(
  une_error *error,
  une_interpreter_state *is,
  une_builtin_type type,
  une_result *args,
  une_node **arg_nodes
);

// GET_IDX Helpers.
static une_result une_interpret_get_idx_list(une_result list, une_int index);
static une_result une_interpret_get_idx_str (une_result str,  une_int index);

#endif /* !UNE_INTERPRETER_H */
