/*
interpreter.c - Une
Modified 2021-07-08
*/

/* Header-specific includes. */
#include "interpreter.h"

/* Implementation-specific includes. */
#include <string.h>
#include <math.h>
#include "tools.h"
#include "types/context.h"

/*
Interpreter function lookup table.
*/
__une_interpreter(*__interpreter_table[]) = {
  &une_interpret_int,
  &une_interpret_flt,
  &une_interpret_str,
  &une_interpret_list,
  &une_interpret_stmts,
  &une_interpret_cop,
  &une_interpret_not,
  &une_interpret_and,
  &une_interpret_or,
  &une_interpret_equ,
  &une_interpret_neq,
  &une_interpret_gtr,
  &une_interpret_geq,
  &une_interpret_lss,
  &une_interpret_leq,
  &une_interpret_add,
  &une_interpret_sub,
  &une_interpret_mul,
  &une_interpret_div,
  &une_interpret_fdiv,
  &une_interpret_mod,
  &une_interpret_pow,
  &une_interpret_neg,
  &une_interpret_set,
  &une_interpret_set_idx,
  &une_interpret_get,
  &une_interpret_get_idx,
  &une_interpret_def,
  &une_interpret_call,
  &une_interpret_for,
  &une_interpret_while,
  &une_interpret_if,
  &une_interpret_continue,
  &une_interpret_break,
  &une_interpret_return,
};

/*
Public interpreter interface.
*/
une_result une_interpret(une_error *error, une_interpreter_state *is, une_node *node)
{
  /* Assert une_node_type can be looked up. */
  UNE_VERIFY_LUT_NODE_TYPE(node->type);
  
  LOGINTERPRET(une_node_type_to_wcs(node->type));
  
  return __interpreter_table[node->type-UNE_R_BGN_LUT_NODES](error, is, node);
}

/*
Interpret a une_node, expecting a specific une_result_type.
*/
__une_interpreter(une_interpret_as, une_result_type type)
{
  une_result result = une_interpret(error, is, node);
  if (result.type != type && result.type != UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    une_result_free(result);
    result = une_result_create(UNE_RT_ERROR);
  }
  return result;
}

/*
Call user-defined function.
*/
__une_static une_result une_interpret_call_def(une_error *error, une_interpreter_state *is, une_function *fn, une_result *args)
{
  /* Create function context. */
  une_context *context = une_context_create(fn->name, UNE_SIZE_VARIABLE_BUF, UNE_SIZE_FUNCTION_BUF);
  context->parent = is->context;
  is->context = context;

  /* Define parameters. */
  for (size_t i=0; i<fn->params_count; i++) {
    une_variable *var = une_variable_create(context, fn->params[i]);
    var->content = une_result_copy(args[i]);
  }

  /* Interpret body. */
  une_result result = une_interpret(error, is, fn->body);
  is->should_return = false;

  /* Return to parent context. */
  is->context = context->parent;
  une_context_free(context);
  return result;
}

/*
Call built-in function.
*/
__une_static une_result une_interpret_call_builtin(une_error *error, une_interpreter_state *is, une_builtin_type type, une_result *args, une_node **arg_nodes)
{
  switch (type) {
    case UNE_BIF_PUT:
      return une_builtin_put(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_PRINT:
      return une_builtin_print(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_TO_INT:
      return une_builtin_to_int(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_TO_FLT:
      return une_builtin_to_flt(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_TO_STR:
      return une_builtin_to_str(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_GET_LEN:
      return une_builtin_get_len(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_SLEEP:
      return une_builtin_sleep(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_CHR:
      return une_builtin_chr(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_ORD:
      return une_builtin_ord(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_READ:
      return une_builtin_read(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_WRITE:
      return une_builtin_write(error, is, arg_nodes[1]->pos, args[0], arg_nodes[2]->pos, args[1]);
    case UNE_BIF_INPUT:
      return une_builtin_input(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_SCRIPT:
      return une_builtin_script(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_EXIST:
      return une_builtin_exist(error, is, arg_nodes[1]->pos, args[0]);
    case UNE_BIF_SPLIT:
      return une_builtin_split(error, is, arg_nodes[1]->pos, args[0], arg_nodes[2]->pos, args[1]);
  }
  
  UNE_VERIFY_NOT_REACHED;
}

/*
Retrieve an item in a UNE_RT_LIST une_result.
*/
__une_static une_result une_interpret_get_idx_list(une_result list, une_int index)
{
  UNE_UNPACK_RESULT_LIST(list, list_p, list_size);

  /* Ensure index is within range. */
  if (index < 0 || index >= list_size)
    return une_result_create(UNE_RT_ERROR);

  return une_result_copy(list_p[1+index]);
}

/*
Retrieve a character in a UNE_RT_STR une_result.
*/
__une_static une_result une_interpret_get_idx_str(une_result str, une_int index)
{
  UNE_UNPACK_NODE_STR(str, string, string_size);

  /* Ensure index is within range. */
  if (index < 0 || index >= string_size)
    return une_result_create(UNE_RT_ERROR);

  wchar_t *substring = malloc(2*sizeof(*substring));
  substring[0] = string[index];
  substring[1] = L'\0';

  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = substring
  };
}

/*
Interpret a UNE_NT_INT une_node.
*/
__une_interpreter(une_interpret_int)
{
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = node->content.value._int
  };
}

/*
Interpret a UNE_NT_FLT une_node.
*/
__une_interpreter(une_interpret_flt)
{
  return (une_result){
    .type = UNE_RT_FLT,
    .value._flt = node->content.value._flt
  };
}

/*
Interpret a UNE_NT_STR une_node.
*/
__une_interpreter(une_interpret_str)
{
  /* DOC: Memory Management: Here we can see that results DUPLICATE strings. */
  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = une_wcsdup(node->content.value._wcs)
  };
}

/*
Interpret a UNE_NT_LIST une_node.
*/
__une_interpreter(une_interpret_list)
{
  UNE_UNPACK_NODE_LIST(node, list, list_size);

  /* Create une_result list. */
  une_result *result_list = une_result_list_create(list_size);

  /* Populate une_result list. */
  UNE_FOR_NODE_LIST_ITEM(i, list_size) {
    result_list[i] = une_interpret(error, is, list[i]);
    if (result_list[i].type == UNE_RT_ERROR) {
      une_result result = une_result_copy(result_list[i]);
      for (size_t j=0; j<=i; j++)
        une_result_free(result_list[j]);
      free(result_list);
      return result;
    }
  }

  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)result_list,
  };
}

/*
Interpret a UNE_NT_STMTS une_node.
*/
__une_interpreter(une_interpret_stmts)
{
  une_result _result; /* Here it's ok to not initialize the result, as it will never be returned like this. */

  UNE_UNPACK_NODE_LIST(node, nodes, nodes_size);

  UNE_FOR_NODE_LIST_ITEM(i, nodes_size) {
    /* Interpret statement. */
    _result = une_interpret(error, is, nodes[i]);
    
    /* Return if required, otherwise discard result. */
    if (_result.type == UNE_RT_ERROR || _result.type == UNE_RT_CONTINUE || _result.type == UNE_RT_BREAK || is->should_return)
      return _result;
    une_result_free(_result);
  }
  
  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_COP une_node.
*/
__une_interpreter(une_interpret_cop)
{
  /* Evaluate condition. */
  une_result condition = une_interpret(error, is, node->content.branch.a);
  if (condition.type == UNE_RT_ERROR)
    return condition;
  
  /* Check if condition applies. */
  une_int is_true = une_result_is_true(condition);
  une_result_free(condition);

  /* Evaluate correct branch. */
  if (is_true)
    return une_interpret(error, is, node->content.branch.b);
  return une_interpret(error, is, node->content.branch.c);
}

/*
Interpret a UNE_NT_NOT une_node.
*/
__une_interpreter(une_interpret_not)
{
  /* Evaluate expression. */
  une_result center = une_interpret(error, is, node->content.branch.a);
  if (center.type == UNE_RT_ERROR)
    return center;
  
  /* Check truth of result. */
  une_int is_true = une_result_is_true(center);
  une_result_free(center);
  
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = !is_true
  };
}

/*
Interpret a UNE_NT_AND une_node.
*/
__une_interpreter(une_interpret_and)
{
  /* Check if branch A is true. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR || !une_result_is_true(left))
    return left;
  une_result_free(left);

  /* Now that we checked branch A, branch B will always hold the outcome of this function. */
  return une_interpret(error, is, node->content.branch.b);
}

/*
Interpret a UNE_NT_OR une_node.
*/
__une_interpreter(une_interpret_or)
{
  /* Check if branch A is true. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR || une_result_is_true(left))
    return left;
  une_result_free(left);

  /* Now that we checked branch A, branch B will always hold the outcome of this function. */
  return une_interpret(error, is, node->content.branch.b);
}

/*
Interpret a UNE_NT_EQU une_node.
*/
__une_interpreter(une_interpret_equ)
{
  /* Evalute branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  /* Check equality. */
  une_int is_equal = une_results_are_equal(left, right);
  une_result_free(left);
  une_result_free(right);
  
  /* Return error if the results can't be compared, otherwise return the result of the comparison. */
  if (is_equal == -1) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = is_equal
  };
}

/*
Interpret a UNE_NT_NEQ une_node.
*/
__une_interpreter(une_interpret_neq)
{
  une_result equ = une_interpret_equ(error, is, node);
  if (equ.type == UNE_RT_ERROR)
    return equ;

  equ.value._int = !equ.value._int;
  return equ;
}

/*
Interpret a UNE_NT_GTR une_node.
*/
__une_interpreter(une_interpret_gtr)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_INT);
  
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT)
    result.value._int = left.value._int > right.value._int;
  else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
    result.value._int = (une_flt)left.value._int > right.value._flt;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
    result.value._int = left.value._flt > (une_flt)right.value._int;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
    result.value._int = left.value._flt > right.value._flt;
  
  /* STR and STR. */
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR)
    result.value._int = wcslen(left.value._wcs) > wcslen(right.value._wcs);
  
  /* LIST and LIST. */
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(left, left_list, left_list_size);
    UNE_UNPACK_RESULT_LIST(right, right_list, right_list_size);
    result.value._int = left_list_size > right_list_size;
  }
  
  /* Illegal Comparison. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_GEQ une_node.
*/
__une_interpreter(une_interpret_geq)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_INT);
  
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT)
    result.value._int = left.value._int >= right.value._int;
  else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
    result.value._int = (une_flt)left.value._int >= right.value._flt;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
    result.value._int = left.value._flt >= (une_flt)right.value._int;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
    result.value._int = left.value._flt >= right.value._flt;
  
  /* STR and STR. */
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR)
    result.value._int = wcslen(left.value._wcs) >= wcslen(right.value._wcs);
  
  /* LIST and LIST. */
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(left, left_list, left_list_size);
    UNE_UNPACK_RESULT_LIST(right, right_list, right_list_size);
    result.value._int = left_list_size >= right_list_size;
  }
  
  /* Illegal Comparison. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_LSS une_node.
*/
__une_interpreter(une_interpret_lss)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_INT);
  
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT)
    result.value._int = left.value._int < right.value._int;
  else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
    result.value._int = (une_flt)left.value._int < right.value._flt;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
    result.value._int = left.value._flt < (une_flt)right.value._int;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
    result.value._int = left.value._flt < right.value._flt;
  
  /* STR and STR. */
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR)
    result.value._int = wcslen(left.value._wcs) < wcslen(right.value._wcs);
  
  /* LIST and LIST. */
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(left, left_list, left_list_size);
    UNE_UNPACK_RESULT_LIST(right, right_list, right_list_size);
    result.value._int = left_list_size < right_list_size;
  }
  
  /* Illegal Comparison. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_LEQ une_node.
*/
__une_interpreter(une_interpret_leq)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_INT);
  
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT)
    result.value._int = left.value._int <= right.value._int;
  else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
    result.value._int = (une_flt)left.value._int <= right.value._flt;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
    result.value._int = left.value._flt <= (une_flt)right.value._int;
  else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
    result.value._int = left.value._flt <= right.value._flt;
  
  /* STR and STR. */
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR)
    result.value._int = wcslen(left.value._wcs) <= wcslen(right.value._wcs);
  
  /* LIST and LIST. */
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(left, left_list, left_list_size);
    UNE_UNPACK_RESULT_LIST(right, right_list, right_list_size);
    result.value._int = left_list_size <= right_list_size;
  }
  
  /* Illegal Comparison. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_ADD une_node.
*/
__une_interpreter(une_interpret_add)
{
  /* Evalute branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(__UNE_RT_none__);
  
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int + right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = (une_flt)left.value._int + right.value._flt;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt + (une_flt)right.value._int;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt + right.value._flt;
  }
  
  /* STR and STR. */
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR)
    result = une_result_strs_add(left, right);
  
  /* LIST and LIST. */
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST)
    result = une_result_lists_add(left, right);

  /* Error. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_SUB une_node.
*/
__une_interpreter(une_interpret_sub)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(__UNE_RT_none__);
  
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int - right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = (une_flt)left.value._int - right.value._flt;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt - (une_flt)right.value._int;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt - right.value._flt;
  }

  /* Error. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_MUL une_node.
*/
__une_interpreter(une_interpret_mul)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(__UNE_RT_none__);
  
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int * right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = (une_flt)left.value._int * right.value._flt;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt * (une_flt)right.value._int;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt * right.value._flt;
  }
  
  /* INT and STR. */
  else if ((left.type == UNE_RT_INT && right.type == UNE_RT_STR) || (left.type == UNE_RT_STR && right.type == UNE_RT_INT)) {
    une_int count;
    une_result wcs;
    if (left.type == UNE_RT_INT) {
      count = left.value._int;
      wcs = right;
    } else {
      count = right.value._int;
      wcs = left;
    }
    if (count < 0)
      count = 0;
    result = une_result_str_mul(wcs, count);
  }
  
  /* INT and LIST. */
  else if ((left.type == UNE_RT_INT && right.type == UNE_RT_LIST) || (left.type == UNE_RT_LIST && right.type == UNE_RT_INT)) {
    une_int count;
    une_result list;
    if (left.type == UNE_RT_INT) {
      count = left.value._int;
      list = right;
    } else {
      count = right.value._int;
      list = left;
    }
    if (count < 0)
      count = 0;
    result = une_result_list_mul(list, count);
  }

  /* Error. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_DIV une_node.
*/
__une_interpreter(une_interpret_div)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(__UNE_RT_none__);
  
  if ((right.type == UNE_RT_INT && right.value._int == 0) || (right.type == UNE_RT_FLT && right.value._flt == 0.0)) {
    *error = UNE_ERROR_SET(UNE_ET_ZERO_DIVISION, node->content.branch.b->pos);
    result.type = UNE_RT_ERROR;
  }
  
  /* INT and FLT. */
  else if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    if (left.value._int % right.value._int == 0) {
      result.type = UNE_RT_INT;
      result.value._int = left.value._int / right.value._int;
    } else {
      result.type = UNE_RT_FLT;
      result.value._flt = (une_flt)left.value._int / right.value._int;
    }
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = (une_flt)left.value._int / right.value._flt;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt / (une_flt)right.value._int;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt / right.value._flt;
  }

  /* Error. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_FDIV une_node.
*/
__une_interpreter(une_interpret_fdiv)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(__UNE_RT_none__);
  
  if ((right.type == UNE_RT_INT && right.value._int == 0) || (right.type == UNE_RT_FLT && right.value._flt == 0.0)) {
    *error = UNE_ERROR_SET(UNE_ET_ZERO_DIVISION, node->content.branch.b->pos);
    result.type = UNE_RT_ERROR;
  }
  
  /* INT and FLT. */
  else if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int / right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = floor((une_flt)left.value._int / right.value._flt);
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_FLT;
    result.value._flt = floor(left.value._flt / (une_flt)right.value._int);
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = floor(left.value._flt / right.value._flt);
  }

  /* Error. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_MOD une_node.
*/
__une_interpreter(une_interpret_mod)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(__UNE_RT_none__);
  
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int % right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = fmod((une_flt)left.value._int, right.value._flt);
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_FLT;
    result.value._flt = fmod(left.value._flt, (une_flt)right.value._int);
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = fmod(left.value._flt, right.value._flt);
  }

  /* Error. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}

/*
Interpret a UNE_NT_POW une_node.
*/
__une_interpreter(une_interpret_pow)
{
  /* Evaluate branches. */
  une_result left = une_interpret(error, is, node->content.branch.a);
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_interpret(error, is, node->content.branch.b);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(__UNE_RT_none__);
  
  /* INT and FLT. */
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_INT;
    result.value._int = (une_int)pow((double)left.value._int, (double)right.value._int);
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = (une_flt)pow((double)left.value._int, (double)right.value._flt);
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.type = UNE_RT_FLT;
    result.value._flt = (une_flt)pow((double)left.value._flt, (double)right.value._int);
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = (une_flt)pow((double)left.value._flt, (double)right.value._flt);
  }
  
  /* Error. */
  else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }

  if (result.type == UNE_RT_FLT && isnan(result.value._flt)) {
    *error = UNE_ERROR_SET(UNE_ET_UNREAL_NUMBER, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  return result;
}

/*
Interpret a UNE_NT_NEG une_node.
*/
__une_interpreter(une_interpret_neg)
{
  /* Evaluate branch. */
  une_result center = une_interpret(error, is, node->content.branch.a);
  if (center.type == UNE_RT_ERROR)
    return center;
  
  une_result result = une_result_create(__UNE_RT_none__);
  
  if (center.type == UNE_RT_INT) {
    result.type = UNE_RT_INT;
    result.value._int = -center.value._int;
  } else if (center.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = -center.value._flt;
  } else {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(center);
  
  return result;
}

/*
Interpret a UNE_NT_SET une_node.
*/
__une_interpreter(une_interpret_set)
{
  /* Evaluate result. */
  une_result result = une_interpret(error, is, node->content.branch.b);
  if (result.type == UNE_RT_ERROR)
    return result;

  /* Get variable name and global. */
  bool global = (bool)node->content.branch.d;
  wchar_t *name = node->content.branch.a->content.value._wcs;

  /* Find (global) or create (local) the variable. */
  une_variable *var;
  if (global)
    var = une_variable_find_or_create_global(is->context, name);
  else
    var = une_variable_find_or_create(is->context, name);
  
  /* Populate variable. */
  une_result_free(var->content);
  var->content = result;

  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_SET_IDX une_node.
*/
__une_interpreter(une_interpret_set_idx)
{
  /* Get name of list and global. */
  bool global = (bool)node->content.branch.d;
  wchar_t *name = node->content.branch.a->content.value._wcs;

  /* Find list in all contexts. */
  une_variable *var;
  if (global)
    var = une_variable_find_global(is->context, name);
  else
    var = une_variable_find(is->context, name);
  if (var == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_VARIABLE_NOT_DEFINED, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  if (var->content.type != UNE_RT_LIST) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  UNE_UNPACK_RESULT_LIST(var->content, list, list_size);

  /* Get index to list. */
  une_result index = une_interpret_as(error, is, node->content.branch.b, UNE_RT_INT);
  if (index.type == UNE_RT_ERROR)
    return index;
  une_int index_value = index.value._int;
  une_result_free(index);
  if (index_value < 0 || index_value >= list_size) {
    *error = UNE_ERROR_SET(UNE_ET_INDEX_OUT_OF_RANGE, node->content.branch.b->pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Set result. */
  une_result result = une_interpret(error, is, node->content.branch.c);
  if (result.type == UNE_RT_ERROR)
    return result;
  une_result_free(list[1+index_value]);
  list[1+index_value] = result;

  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_GET une_node.
*/
__une_interpreter(une_interpret_get)
{
  /* Get name of variable. */
  wchar_t *name = node->content.branch.a->content.value._wcs;

  /* Find variable in all contexts. */
  une_variable *var = une_variable_find_global(is->context, name);
  if (var == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_VARIABLE_NOT_DEFINED, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }

  return une_result_copy(var->content);
}

/*
Interpret a UNE_NT_GET_IDX une_node.
*/
__une_interpreter(une_interpret_get_idx)
{
  /* Get base. */
  une_result base = une_interpret(error, is, node->content.branch.a);
  if (base.type == UNE_RT_ERROR)
    return base;

  /* Get index. */
  une_result index_result = une_interpret_as(error, is, node->content.branch.b, UNE_RT_INT);
  if (index_result.type == UNE_RT_ERROR) {
    une_result_free(base);
    return index_result;
  }
  une_int index = index_result.value._int;
  une_result_free(index_result);

  /* Get index of base. */
  une_result result;

  switch (base.type) {
    
    case UNE_RT_LIST:
      result = une_interpret_get_idx_list(base, index);
      une_result_free(base);
      if (result.type == UNE_RT_ERROR)
        break;
      return result;

    case UNE_RT_STR:
      result = une_interpret_get_idx_str(base, index);
      une_result_free(base);
      if (result.type == UNE_RT_ERROR)
        break;
      return result;

    /* Type not indexable. */
    default:
      une_result_free(base);
      *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
      return une_result_create(UNE_RT_VOID);
  
  }

  /* Index out of range (une_interpret_get_idx_TYPE returned UNE_RT_ERROR). */
  *error = UNE_ERROR_SET(UNE_ET_INDEX_OUT_OF_RANGE, node->content.branch.b->pos);
  return une_result_create(UNE_RT_ERROR);
}

/*
Interpret a UNE_NT_DEF une_node.
*/
__une_interpreter(une_interpret_def)
{
  /* Get function name. */
  wchar_t *name = node->content.branch.a->content.value._wcs;
  
  /* Check if function already exists *in current context*. */
  une_function *fn = une_function_find(is->context, name);
  if (fn != NULL) {
    *error = UNE_ERROR_SET(UNE_ET_FUNCTION_ALREADY_DEFINED, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Copy contents. */
  UNE_UNPACK_NODE_LIST(node->content.branch.b, params_n, params_count);
  wchar_t **params = NULL;
  if (params_count > 0)
    params = malloc(params_count*sizeof(*params));
  for (size_t i=0; i<params_count; i++)
    params[i] = une_wcsdup(params_n[i+1]->content.value._wcs);
  une_node *body = une_node_copy(node->content.branch.c);

  /* Define function. */
  fn = une_function_create(is->context, name);
  fn->params_count = params_count;
  fn->params = params;
  fn->body = body;
  
  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_CALL une_node.
*/
__une_interpreter(une_interpret_call)
{
  /* Get function name. */
  wchar_t *name = node->content.branch.a->content.value._wcs;

  size_t num_of_params;
  une_function *fn;

  /* Check if function is built-in. */
  une_builtin_type builtin_type = une_builtin_wcs_to_type(name);
  if (builtin_type != __UNE_BIF_none__)
    num_of_params = une_builtin_get_num_of_params(builtin_type);

  /* Check if function exists in symbol table. */
  else {
    fn = une_function_find_global(is->context, name);
    if (fn == NULL) {
      *error = UNE_ERROR_SET(UNE_ET_FUNCTION_NOT_DEFINED, node->content.branch.a->pos);
      return une_result_create(UNE_RT_ERROR);
    }
    num_of_params = fn->params_count;
  }

  /* Get arguments and compare number of arguments to number of parameters. */
  UNE_UNPACK_NODE_LIST(node->content.branch.b, args_n, args_count);
  if (args_count != num_of_params) {
    *error = UNE_ERROR_SET(UNE_ET_FUNCTION_ARGC, node->content.branch.b->pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Interpret arguments. */
  une_result *args = NULL;
  if (args_count > 0)
    args = malloc(args_count*sizeof(*args));
  for (size_t i=0; i<args_count; i++) {
    une_result temp = une_interpret(error, is, args_n[i+1]);
    if (temp.type == UNE_RT_ERROR) {
      for (size_t j=0; j<i; j++)
        une_result_free(args[j]);
      free(args);
      return temp;
    }
    args[i] = temp;
  }

  /* Execute function. */
  une_result result;
  if (builtin_type != __UNE_BIF_none__)
    result = une_interpret_call_builtin(error, is, builtin_type, args, args_n);
  else
    result = une_interpret_call_def(error, is, fn, args);
  for (size_t i=0; i<num_of_params; i++)
    une_result_free(args[i]);
  if (args != NULL)
    free(args);
  return result;
}

/*
Interpret a UNE_NT_FOR une_node.
*/
__une_interpreter(une_interpret_for)
{
  /* Get range. */
  une_result result = une_interpret_as(error, is, node->content.branch.b, UNE_RT_INT);
  if (result.type == UNE_RT_ERROR)
    return result;
  une_int from = result.value._int;
  une_result_free(result);
  result = une_interpret_as(error, is, node->content.branch.c, UNE_RT_INT);
  if (result.type == UNE_RT_ERROR)
    return result;
  une_int till = result.value._int;
  une_result_free(result);
  if (from == till)
    return une_result_create(UNE_RT_VOID);
  
  /* Determine step. */
  une_int step;
  if (from < till)
    step = 1;
  else
    step = -1;

  /* Get loop variable name. */
  wchar_t *id = node->content.branch.a->content.value._wcs;
  une_variable *var = une_variable_find_or_create(is->context, id); /* We only check the *local* variables. */

  for (une_int i=from; i!=till; i+=step) {
    une_result_free(var->content);
    var->content = (une_result){
      .type = UNE_RT_INT,
      .value._int = i
    };
    result = une_interpret(error, is, node->content.branch.d);
    if (result.type == UNE_RT_ERROR || is->should_return)
      return result;
    if (result.type == UNE_RT_BREAK)
      break;
    if (i+step != till)
      une_result_free(result);
  }
  
  une_result_free(result);
  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_WHILE une_node.
*/
__une_interpreter(une_interpret_while)
{
  une_result result, condition;
  une_result_type result_type;
  une_int condition_is_true;

  while (true) {
    condition = une_interpret(error, is, node->content.branch.a);
    if (condition.type == UNE_RT_ERROR)
      return condition;
    condition_is_true = une_result_is_true(condition);
    une_result_free(condition);
    if (!condition_is_true)
      break;
    result = une_interpret(error, is, node->content.branch.b);
    if (result.type == UNE_RT_ERROR || is->should_return)
      return result;
    result_type = result.type;
    une_result_free(result);
    if (result_type == UNE_RT_BREAK)
      break;
  }

  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_IF une_node.
*/
__une_interpreter(une_interpret_if)
{
  /* Check if predicate applies. */
  une_result predicate = une_interpret(error, is, node->content.branch.a);
  if (predicate.type == UNE_RT_ERROR)
    return predicate;
  une_int is_true = une_result_is_true(predicate);
  une_result_free(predicate);
  
  if (is_true)
    return une_interpret(error, is, node->content.branch.b);
  if (node->content.branch.c != NULL)
    return une_interpret(error, is, node->content.branch.c);
  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_CONTINUE une_node.
*/
__une_interpreter(une_interpret_continue)
{
  return une_result_create(UNE_RT_CONTINUE);
}

/*
Interpret a UNE_NT_BREAK une_node.
*/
__une_interpreter(une_interpret_break)
{
  return une_result_create(UNE_RT_BREAK);
}

/*
Interpret a UNE_NT_RETURN une_node.
*/
__une_interpreter(une_interpret_return)
{
  if (node->content.branch.a == NULL)
    return une_result_create(UNE_RT_VOID);
  une_result result = une_interpret(error, is, node->content.branch.a);
  is->should_return = true;
  return result;
}
