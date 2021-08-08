/*
interpreter.c - Une
Modified 2021-08-08
*/

/* Header-specific includes. */
#include "interpreter.h"

/* Implementation-specific includes. */
#include <string.h>
#include <math.h>
#include "tools.h"
#include "types/context.h"
#include "datatypes/datatypes.h"

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
  assert(UNE_NODE_TYPE_IS_IN_LUT(node->type));
  
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
  une_context *parent = is->context;
  is->context = une_context_create(fn->name, UNE_SIZE_VARIABLE_BUF, UNE_SIZE_FUNCTION_BUF);
  is->context->parent = parent;

  /* Define parameters. */
  for (size_t i=0; i<fn->params_count; i++) {
    une_variable *var = une_variable_create(is->context, fn->params[i]);
    var->content = une_result_copy(args[i]);
  }

  /* Interpret body. */
  une_result result = une_interpret(error, is, fn->body);
  is->should_return = false;

  /* Return to parent context. */
  if (result.type != UNE_RT_ERROR) {
    une_context_free_children(parent, is->context);
    is->context = parent;
  }
  return result;
}

/*
Call built-in function.
*/
__une_static une_result une_interpret_call_builtin(une_error *error, une_interpreter_state *is, une_node *call_node, une_result *args, une_builtin_fnptr fn)
{
  return (*fn)(error, is, call_node, args);
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
    .value._wcs = wcsdup(node->content.value._wcs)
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
  
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  assert(dt_left.is_greater != NULL);
  une_int is_greater = dt_left.is_greater(left, right);
  une_result_free(left);
  une_result_free(right);
  
  if (is_greater == -1) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = is_greater
  };
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
  
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  assert(dt_left.is_greater_or_equal != NULL);
  une_int is_greater_or_equal = dt_left.is_greater_or_equal(left, right);
  une_result_free(left);
  une_result_free(right);
  
  if (is_greater_or_equal == -1) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = is_greater_or_equal
  };
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
  
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  assert(dt_left.is_less != NULL);
  une_int is_less = dt_left.is_less(left, right);
  une_result_free(left);
  une_result_free(right);
  
  if (is_less == -1) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = is_less
  };
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
  
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  assert(dt_left.is_less_or_equal != NULL);
  une_int is_less_or_equal = dt_left.is_less_or_equal(left, right);
  une_result_free(left);
  une_result_free(right);
  
  if (is_less_or_equal == -1) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = is_less_or_equal
  };
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
  
  une_result sum = une_result_create(__UNE_RT_none__);
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  if (dt_left.add != NULL)
    sum = dt_left.add(left, right);
  une_result_free(left);
  une_result_free(right);
  if (dt_left.add == NULL || sum.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    sum = une_result_create(UNE_RT_ERROR);
  }
  return sum;
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
  
  une_result difference = une_result_create(__UNE_RT_none__);
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  if (dt_left.sub != NULL)
    difference = dt_left.sub(left, right);
  une_result_free(left);
  une_result_free(right);
  if (dt_left.sub == NULL || difference.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    difference = une_result_create(UNE_RT_ERROR);
  }
  return difference;
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
  
  une_result product = une_result_create(__UNE_RT_none__);
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  if (dt_left.mul != NULL)
    product = dt_left.mul(left, right);
  une_result_free(left);
  une_result_free(right);
  if (dt_left.mul == NULL || product.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    product = une_result_create(UNE_RT_ERROR);
  }
  return product;
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
  
  une_result quotient = une_result_create(__UNE_RT_none__);
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  if (dt_left.div != NULL)
    quotient = dt_left.div(left, right);
  une_result_free(left);
  une_result_free(right);
  if (dt_left.div == NULL || quotient.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    quotient = une_result_create(UNE_RT_ERROR);
  }
  if (quotient.type == UNE_RT_FLT && quotient.value._flt == INFINITY) {
    /* Zero division. */
    *error = UNE_ERROR_SET(UNE_ET_ZERO_DIVISION, node->pos);
    quotient = une_result_create(UNE_RT_ERROR);
  }
  return quotient;
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
  
  une_result quotient = une_result_create(__UNE_RT_none__);
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  if (dt_left.fdiv != NULL)
    quotient = dt_left.fdiv(left, right);
  une_result_free(left);
  une_result_free(right);
  if (dt_left.fdiv == NULL || quotient.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    quotient = une_result_create(UNE_RT_ERROR);
  }
  if (quotient.type == UNE_RT_FLT && quotient.value._flt == INFINITY) {
    /* Zero division. */
    *error = UNE_ERROR_SET(UNE_ET_ZERO_DIVISION, node->pos);
    quotient = une_result_create(UNE_RT_ERROR);
  }
  return quotient;
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
  
  une_result remainder = une_result_create(__UNE_RT_none__);
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  if (dt_left.mod != NULL)
    remainder = dt_left.mod(left, right);
  une_result_free(left);
  une_result_free(right);
  if (dt_left.mod == NULL || remainder.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    remainder = une_result_create(UNE_RT_ERROR);
  }
  return remainder;
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
  
  une_result raised = une_result_create(__UNE_RT_none__);
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  if (dt_left.pow != NULL)
    raised = dt_left.pow(left, right);
  une_result_free(left);
  une_result_free(right);
  if (dt_left.pow == NULL || raised.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    raised = une_result_create(UNE_RT_ERROR);
  }
  if (raised.type == UNE_RT_FLT && isnan(raised.value._flt)) {
    /* Unreal number. */
    *error = UNE_ERROR_SET(UNE_ET_UNREAL_NUMBER, node->pos);
    raised = une_result_create(UNE_RT_ERROR);
  }
  return raised;
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
  
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(center);
  if (dt_left.negate == NULL) {
    une_result_free(center);
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  une_result negative = dt_left.negate(center);
  une_result_free(center);
  return negative;
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
  /* Get name of variable and global. */
  bool global = (bool)node->content.branch.d;
  wchar_t *name = node->content.branch.a->content.value._wcs;

  /* Find variable in all contexts. */
  une_variable *var;
  if (global)
    var = une_variable_find_global(is->context, name);
  else
    var = une_variable_find(is->context, name);
  if (var == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_SYMBOL_NOT_DEFINED, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  une_datatype dt_var = UNE_DATATYPE_FOR_RESULT(var->content);
  if (dt_var.set_index == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Get index to variable and check if it is valid. */
  une_result index = une_interpret(error, is, node->content.branch.b);
  assert(dt_var.is_valid_index_type != NULL);
  if (!dt_var.is_valid_index_type(index.type)) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
  } else {
    assert(dt_var.is_valid_index != NULL);
    if (!dt_var.is_valid_index(var->content, index))
      *error = UNE_ERROR_SET(UNE_ET_INDEX_OUT_OF_RANGE, node->content.branch.b->pos);
  }
  if (error->type != __UNE_ET_none__) {
    une_result_free(index);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Set index. */
  une_result result = une_interpret(error, is, node->content.branch.c);
  if (result.type == UNE_RT_ERROR)
    return result;
  dt_var.set_index(&var->content, index, result);
  une_result_free(index);
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
    *error = UNE_ERROR_SET(UNE_ET_SYMBOL_NOT_DEFINED, node->content.branch.a->pos);
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
  une_datatype dt_base = UNE_DATATYPE_FOR_RESULT(base);
  if (dt_base.get_index == NULL) {
    une_result_free(base);
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Get index. */
  une_result index = une_interpret(error, is, node->content.branch.b);
  if (index.type == UNE_RT_ERROR) {
    une_result_free(base);
    return index;
  }
  assert(dt_base.is_valid_index_type != NULL);
  if (!dt_base.is_valid_index_type(index.type)) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
  } else {
    assert(dt_base.is_valid_index != NULL);
    if (!dt_base.is_valid_index(base, index))
      *error = UNE_ERROR_SET(UNE_ET_INDEX_OUT_OF_RANGE, node->content.branch.b->pos);
  }
  if (error->type != __UNE_ET_none__) {
    une_result_free(base);
    une_result_free(index);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Get index of base. */
  assert(dt_base.get_index != NULL);
  une_result result = dt_base.get_index(base, index);
  une_result_free(base);
  une_result_free(index);
  return result;
}

/*
Interpret a UNE_NT_DEF une_node.
*/
__une_interpreter(une_interpret_def)
{
  /* Get function name. */
  wchar_t *name = node->content.branch.a->content.value._wcs;
  
  /* Check if function built-in or already exists *in current context*. */
  if (une_builtin_wcs_to_fnptr(name) != NULL || une_function_find(is->context, name) != NULL) {
    *error = UNE_ERROR_SET(UNE_ET_FUNCTION_ALREADY_DEFINED, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Copy contents. */
  UNE_UNPACK_NODE_LIST(node->content.branch.b, params_n, params_count);
  wchar_t **params = NULL;
  if (params_count > 0)
    params = malloc(params_count*sizeof(*params));
  for (size_t i=0; i<params_count; i++)
    params[i] = wcsdup(params_n[i+1]->content.value._wcs);
  une_node *body = une_node_copy(node->content.branch.c);

  /* Define function. */
  une_function *fn = une_function_create(is->context, name);
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

  /* Unpack arguments. */
  UNE_UNPACK_NODE_LIST(node->content.branch.b, args_n, args_count);

  size_t params_count;

  /* Check if function is built-in. */
  une_builtin_fnptr builtin_fn = une_builtin_wcs_to_fnptr(name);
  if (builtin_fn != NULL)
    params_count = une_builtin_params_count(builtin_fn);

  /* Check if function exists in symbol table. */
  une_function *fn;
  if (builtin_fn == NULL) {
    fn = une_function_find_global(is->context, name);
    if (fn == NULL) {
      *error = UNE_ERROR_SET(UNE_ET_SYMBOL_NOT_DEFINED, node->content.branch.a->pos);
      return une_result_create(UNE_RT_ERROR);
    }
    params_count = fn->params_count;
  }
  
  /* Compare number of arguments to number of parameters. */
  if (args_count != params_count) {
    *error = UNE_ERROR_SET(UNE_ET_FUNCTION_ARG_COUNT, node->content.branch.b->pos);
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
  if (builtin_fn == NULL)
    result = une_interpret_call_def(error, is, fn, args);
  else
    result = une_interpret_call_builtin(error, is, node, args, builtin_fn);
  
  /* Clean up. */
  for (size_t i=0; i<args_count; i++)
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
  
  /* Get loop variable. */
  wchar_t *id = node->content.branch.a->content.value._wcs;
  une_variable *var = une_variable_find_or_create(is->context, id); /* We only check the *local* variables. */
  
  /* Loop. */
  for (une_int i=from; i!=till; i+=step) {
    une_result_free(var->content);
    var->content = (une_result){
      .type = UNE_RT_INT,
      .value._int = i
    };
    result = une_interpret(error, is, node->content.branch.d);
    if (result.type == UNE_RT_ERROR || is->should_return)
      return result;
    if (result.type == UNE_RT_BREAK) {
      une_result_free(result);
      break;
    }
    une_result_free(result);
  }
  
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
