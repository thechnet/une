/*
interpreter.c - Une
Modified 2023-02-14
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
une_interpreter__(*interpreter_table__[]) = {
  &une_interpret_void,
  &une_interpret_int,
  &une_interpret_flt,
  &une_interpret_str,
  &une_interpret_list,
  &une_interpret_object,
  &une_interpret_function,
  &une_interpret_builtin,
  &une_interpret_stmts,
  &une_interpret_cop,
  &une_interpret_not,
  &une_interpret_and,
  &une_interpret_or,
  &une_interpret_nullish,
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
  NULL, /* une_interpret_seek. */
  &une_interpret_idx_seek,
  &une_interpret_member_seek,
  &une_interpret_assign,
  &une_interpret_get,
  &une_interpret_idx_get,
  &une_interpret_member_get,
  &une_interpret_call,
  &une_interpret_for_range,
  &une_interpret_for_element,
  &une_interpret_while,
  &une_interpret_if,
  &une_interpret_continue,
  &une_interpret_break,
  &une_interpret_return,
  &une_interpret_exit,
  &une_interpret_cover,
  &une_interpret_concatenate,
  &une_interpret_this,
};

/*
Public interpreter interface.
*/
une_result une_interpret(une_error *error, une_interpreter_state *is, une_node *node)
{
  assert(UNE_NODE_TYPE_IS_IN_LUT(node->type));
  
  LOGINTERPRET(une_node_type_to_wcs(node->type));
  
  return interpreter_table__[(node->type)-UNE_R_BGN_LUT_NODES](error, is, node);
}

/*
Interpret a une_node, expecting a specific une_result_type.
*/
une_interpreter__(une_interpret_as, une_result_type type)
{
  une_result result = une_result_dereference(une_interpret(error, is, node));
  if (result.type != type && result.type != UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    une_result_free(result);
    result = une_result_create(UNE_RT_ERROR);
  }
  return result;
}

/*
Interpret a UNE_NT_VOID une_node.
*/
une_interpreter__(une_interpret_void)
{
  return (une_result){
    .type = UNE_RT_VOID,
    .value._int = node->content.value._int
  };
}

/*
Interpret a UNE_NT_INT une_node.
*/
une_interpreter__(une_interpret_int)
{
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = node->content.value._int
  };
}

/*
Interpret a UNE_NT_FLT une_node.
*/
une_interpreter__(une_interpret_flt)
{
  return (une_result){
    .type = UNE_RT_FLT,
    .value._flt = node->content.value._flt
  };
}

/*
Interpret a UNE_NT_STR une_node.
*/
une_interpreter__(une_interpret_str)
{
  /* DOC: Memory Management: Here we can see that results DUPLICATE strings. */
  une_result result = {
    .type = UNE_RT_STR,
    .value._wcs = wcsdup(node->content.value._wcs)
  };
  verify(result.value._wcs);
  return result;
}

/*
Interpret a UNE_NT_LIST une_node.
*/
une_interpreter__(une_interpret_list)
{
  UNE_UNPACK_NODE_LIST(node, list, list_size);

  /* Create une_result list. */
  une_result *result_list = une_result_list_create(list_size);

  /* Populate une_result list. */
  UNE_FOR_NODE_LIST_ITEM(i, list_size) {
    result_list[i] = une_result_dereference(une_interpret(error, is, list[i]));
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
Interpret a UNE_NT_OBJECT une_node.
*/
une_interpreter__(une_interpret_object)
{
  UNE_UNPACK_NODE_LIST(node, list, list_size);

  /* Create object. */
  une_object *object = malloc(sizeof(*object));
  verify(object);
  object->members_length = list_size;
  object->members = malloc(object->members_length*sizeof(*object->members));
  verify(object->members);

  /* Store associations. */
  UNE_FOR_NODE_LIST_ITEM(i, list_size) {
    /* Add association. */
    une_association *association = malloc(sizeof(*association));
    verify(association);
    object->members[i-1] = association;
    /* Populate association. */
    association->name = wcsdup(list[i]->content.branch.a->content.value._wcs);
    verify(association->name);
    association->content = une_result_dereference(une_interpret(error, is, list[i]->content.branch.b));
    if (association->content.type == UNE_RT_ERROR) {
      une_result result = une_result_copy(association->content);
      UNE_FOR_NODE_LIST_ITEM(j, i) {
        free(object->members[j-1]->name);
        une_result_free(object->members[j-1]->content);
        free(object->members[j-1]);
      }
      free(object->members);
      free(object);
      return result;
    }
  }
  return (une_result){
    .type = UNE_RT_OBJECT,
    .value._vp = (void*)object,
  };
}

/*
Interpret a UNE_NT_FUNCTION une_node.
*/
une_interpreter__(une_interpret_function)
{
  UNE_UNPACK_NODE_LIST(node->content.branch.a, params_n, params_count);
  size_t function = une_function_create(is, (char*)node->content.branch.c, node->pos);
  wchar_t **params = NULL;
  if (params_count > 0) {
    params = malloc(params_count*sizeof(*params));
    verify(params);
  }
  for (size_t i=0; i<params_count; i++) {
    params[i] = wcsdup(params_n[i+1]->content.value._wcs);
    verify(params[i]);
  }
  (is->functions)[function].params = params;
  (is->functions)[function].params_count = params_count;
  (is->functions)[function].body = une_node_copy(node->content.branch.b);
  return (une_result){
    .type = UNE_RT_FUNCTION,
    .value._int = (une_int)function
  };
}

/*
Interpret a UNE_NT_BUILTIN une_node.
*/
une_interpreter__(une_interpret_builtin)
{
  return (une_result){
    .type = UNE_RT_BUILTIN,
    .value._int = node->content.value._int
  };
}

/*
Interpret a UNE_NT_STMTS une_node.
*/
une_interpreter__(une_interpret_stmts)
{
  une_result result_ = une_result_create(UNE_RT_VOID);

  UNE_UNPACK_NODE_LIST(node, nodes, nodes_size);

  UNE_FOR_NODE_LIST_ITEM(i, nodes_size) {
    /* Free previous result. */
    une_result_free(result_);
    
    /* Interpret statement. */
    result_ = une_result_dereference(une_interpret(error, is, nodes[i]));
    
    /* Return if required. */
    if (result_.type == UNE_RT_ERROR || result_.type == UNE_RT_CONTINUE || result_.type == UNE_RT_BREAK || is->should_return || is->should_exit)
      return result_;
  }
  
  return result_; /* Return last result. */
}

/*
Interpret a UNE_NT_COP une_node.
*/
une_interpreter__(une_interpret_cop)
{
  /* Evaluate condition. */
  une_result condition = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (condition.type == UNE_RT_ERROR)
    return condition;
  
  /* Check if condition applies. */
  une_int is_true = une_result_is_true(condition);
  une_result_free(condition);

  /* Evaluate correct branch. */
  if (is_true)
    return une_result_dereference(une_interpret(error, is, node->content.branch.b));
  return une_result_dereference(une_interpret(error, is, node->content.branch.c));
}

/*
Interpret a UNE_NT_NOT une_node.
*/
une_interpreter__(une_interpret_not)
{
  /* Evaluate expression. */
  une_result center = une_result_dereference(une_interpret(error, is, node->content.branch.a));
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
une_interpreter__(une_interpret_and)
{
  /* Check if branch A is true. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR || !une_result_is_true(left))
    return left;
  une_result_free(left);

  /* Now that we checked branch A, branch B will always hold the outcome of this function. */
  return une_result_dereference(une_interpret(error, is, node->content.branch.b));
}

/*
Interpret a UNE_NT_OR une_node.
*/
une_interpreter__(une_interpret_or)
{
  /* Check if branch A is true. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR || une_result_is_true(left))
    return left;
  une_result_free(left);

  /* Now that we checked branch A, branch B will always hold the outcome of this function. */
  return une_result_dereference(une_interpret(error, is, node->content.branch.b));
}

/*
Interpret a UNE_NT_NULLISH une_node.
*/
une_interpreter__(une_interpret_nullish)
{
  /* Check if branch A is not VOID. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type != UNE_RT_VOID)
    return left;
  une_result_free(left);

  /* Now that we checked branch A, branch B will always hold the outcome of this function. */
  return une_result_dereference(une_interpret(error, is, node->content.branch.b));
}

/*
Interpret a UNE_NT_EQU une_node.
*/
une_interpreter__(une_interpret_equ)
{
  /* Evalute branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
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
une_interpreter__(une_interpret_neq)
{
  une_result equ = une_result_dereference(une_interpret_equ(error, is, node));
  if (equ.type == UNE_RT_ERROR)
    return equ;

  equ.value._int = !equ.value._int;
  return equ;
}

/*
Interpret a UNE_NT_GTR une_node.
*/
une_interpreter__(une_interpret_gtr)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
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
une_interpreter__(une_interpret_geq)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
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
une_interpreter__(une_interpret_lss)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
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
une_interpreter__(une_interpret_leq)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
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
une_interpreter__(une_interpret_add)
{
  /* Evalute branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result sum = une_result_create(UNE_RT_none__);
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
une_interpreter__(une_interpret_sub)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result difference = une_result_create(UNE_RT_none__);
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
une_interpreter__(une_interpret_mul)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result product = une_result_create(UNE_RT_none__);
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
une_interpreter__(une_interpret_div)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result quotient = une_result_create(UNE_RT_none__);
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  if (dt_left.div != NULL)
    quotient = dt_left.div(left, right);
  une_result_free(left);
  une_result_free(right);
  if (dt_left.div == NULL || quotient.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    quotient = une_result_create(UNE_RT_ERROR);
  }
  if (quotient.type == UNE_RT_FLT && isinf(quotient.value._flt)) {
    /* Zero division. */
    *error = UNE_ERROR_SET(UNE_ET_ZERO_DIVISION, node->pos);
    quotient = une_result_create(UNE_RT_ERROR);
  }
  return quotient;
}

/*
Interpret a UNE_NT_FDIV une_node.
*/
une_interpreter__(une_interpret_fdiv)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result quotient = une_result_create(UNE_RT_none__);
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  if (dt_left.fdiv != NULL)
    quotient = dt_left.fdiv(left, right);
  une_result_free(left);
  une_result_free(right);
  if (dt_left.fdiv == NULL || quotient.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
    quotient = une_result_create(UNE_RT_ERROR);
  }
  if (quotient.type == UNE_RT_FLT && isinf(quotient.value._flt)) {
    /* Zero division. */
    *error = UNE_ERROR_SET(UNE_ET_ZERO_DIVISION, node->pos);
    quotient = une_result_create(UNE_RT_ERROR);
  }
  return quotient;
}

/*
Interpret a UNE_NT_MOD une_node.
*/
une_interpreter__(une_interpret_mod)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result remainder = une_result_create(UNE_RT_none__);
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
une_interpreter__(une_interpret_pow)
{
  /* Evaluate branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result raised = une_result_create(UNE_RT_none__);
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
une_interpreter__(une_interpret_neg)
{
  /* Evaluate branch. */
  une_result center = une_result_dereference(une_interpret(error, is, node->content.branch.a));
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
Interpret a UNE_NT_SEEK une_node.
*/
une_interpreter__(une_interpret_seek, bool existing_only, bool force_global)
{
  /* Extract information. */
  wchar_t *name = node->content.branch.a->content.value._wcs;
  bool global = (une_node*)node->content.branch.b || force_global;
  
  /* Find variable. */
  une_association *var;
  if (global) {
    if (existing_only)
      var = une_variable_find_global(is->context, name);
    else
      var = une_variable_find_or_create_global(is->context, name);
  } else {
    if (existing_only)
      var = une_variable_find(is->context, name);
    else
      var = une_variable_find_or_create(is->context, name);
  }
  if (var == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_SYMBOL_NOT_DEFINED, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Return pointer to result container. */
  une_result result = une_result_create(UNE_RT_GENERIC_REFERENCE);
  result.value._vp = (void*)&var->content;
  return result;
}

/*
Interpret a UNE_NT_ASSIGN une_node.
*/
une_interpreter__(une_interpret_assign)
{
  /* Evaluate assignee. */
  une_result assignee;
  if (node->content.branch.a->type == UNE_NT_SEEK)
    assignee = une_interpret_seek(error, is, node->content.branch.a, false, false);
  else
    assignee = une_interpret(error, is, node->content.branch.a);
  if (assignee.type == UNE_RT_ERROR)
    return assignee;
  
  /* Evaluate result. */
  une_result result = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (result.type == UNE_RT_ERROR) {
    une_result_free(assignee);
    return result;
  }
  
  /* Check if result is valid element. */
  une_datatype dt_assignee = UNE_DATATYPE_FOR_RESULT_TYPE(assignee.type == UNE_RT_GENERIC_REFERENCE ? UNE_RT_LIST : UNE_RT_STR);
  assert(dt_assignee.is_valid_element);
  if (!dt_assignee.is_valid_element(result)) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
    une_result_free(assignee);
    une_result_free(result);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Assign. */
  if (assignee.type == UNE_RT_GENERIC_REFERENCE) {
    une_result *target = (une_result*)assignee.value._vp;
    une_result_free(*target);
    *target = result; /* Instead of copying this result, we just use the original; this way, we also don't need to worry about freeing it. */
  } else if (assignee.type == UNE_RT_STR_ELEMENT_REFERENCE) {
    *assignee.value._wcs = result.value._wcs[0];
    une_result_free(result);
  }
  une_result_free(assignee);
  
  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_IDX_SEEK une_node.
*/
une_interpreter__(une_interpret_idx_seek)
{
  /* Evaluate container. */
  une_result container;
  if (node->content.branch.a->type == UNE_NT_SEEK)
    container = une_interpret_seek(error, is, node->content.branch.a, true, false);
  else
    container = une_interpret(error, is, node->content.branch.a);
  if (container.type == UNE_RT_ERROR)
    return container;
  
  /* Fail if container cannot be indexed into. */
  if (container.type != UNE_RT_GENERIC_REFERENCE) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    une_result_free(container);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Access contained result. */
  assert(container.value._vp);
  une_result *result = (une_result*)container.value._vp;
  
  /* Check if container result supports indexing. */
  une_datatype dt_result = UNE_DATATYPE_FOR_RESULT(*result);
  if (!dt_result.seek_index) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    une_result_free(container);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Evaluate index. */
  une_result index = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (index.type == UNE_RT_ERROR)
    return index;
  
  /* Check if index type is valid. */
  assert(dt_result.is_valid_index_type);
  if (!dt_result.is_valid_index_type(index.type)) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
    une_result_free(container);
    une_result_free(index);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Check if index is valid. */
  assert(dt_result.is_valid_index);
  if (!dt_result.is_valid_index(*result, index)) {
    *error = UNE_ERROR_SET(UNE_ET_INDEX_OUT_OF_RANGE, node->content.branch.b->pos);
    une_result_free(container);
    une_result_free(index);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Return seeked index. */
  assert(dt_result.seek_index);
  return dt_result.seek_index(result, index);
}

/*
Interpret a UNE_NT_MEMBER_SEEK une_node.
*/
une_interpreter__(une_interpret_member_seek)
{
  /* Evaluate container. */
  une_result container;
  if (node->content.branch.a->type == UNE_NT_SEEK)
    container = une_interpret_seek(error, is, node->content.branch.a, true, false);
  else
    container = une_interpret(error, is, node->content.branch.a);
  if (container.type == UNE_RT_ERROR)
    return container;
  
  /* Fail if container cannot contain members. */
  if (container.type != UNE_RT_GENERIC_REFERENCE) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    une_result_free(container);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Access contained result. */
  assert(container.value._vp);
  une_result *result = (une_result*)container.value._vp;
  
  /* Check if container result supports members. */
  une_datatype dt_result = UNE_DATATYPE_FOR_RESULT(*result);
  if (!dt_result.seek_member) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    une_result_free(container);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Extract member name. */
  assert(node->content.branch.b->type == UNE_NT_ID);
  wchar_t *name = node->content.branch.b->content.value._wcs;
  
  /* Seek member. */
  assert(dt_result.member_exists && dt_result.add_member);
  if (dt_result.member_exists(*result, name))
    return dt_result.seek_member(result, name);
  return dt_result.add_member(result, name);
}

/*
Interpret a UNE_NT_GET une_node.
*/
une_interpreter__(une_interpret_get)
{
  return une_interpret_seek(error, is, node, true, true);
  
  // /* Get name of variable. */
  // wchar_t *name = node->content.branch.a->content.value._wcs;

  // /* Find variable in all contexts. */
  // une_association *var = une_variable_find_global(is->context, name);
  // if (var == NULL) {
  //   *error = UNE_ERROR_SET(UNE_ET_SYMBOL_NOT_DEFINED, node->content.branch.a->pos);
  //   return une_result_create(UNE_RT_ERROR);
  // }

  // return une_result_copy(var->content);
}

/*
Interpret a UNE_NT_IDX_GET une_node.
*/
une_interpreter__(une_interpret_idx_get)
{
  /* Get base. */
  une_result base = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (base.type == UNE_RT_ERROR)
    return base;
  une_datatype dt_base = UNE_DATATYPE_FOR_RESULT(base);
  if (dt_base.get_index == NULL) {
    une_result_free(base);
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Get index. */
  une_result index = une_result_dereference(une_interpret(error, is, node->content.branch.b));
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
  if (error->type != UNE_ET_none__) {
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
Interpret a UNE_NT_MEMBER_GET une_node.
*/
une_interpreter__(une_interpret_member_get)
{
  /* Get base. */
  une_result base_raw = une_interpret(error, is, node->content.branch.a);
  une_result base = une_result_dereference(base_raw);
  if (base.type == UNE_RT_ERROR)
    return base;
  une_datatype dt_base = UNE_DATATYPE_FOR_RESULT(base);
  if (dt_base.get_member == NULL) {
    une_result_free(base);
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Extract member name. */
  assert(node->content.branch.b->type == UNE_NT_ID);
  wchar_t *name = node->content.branch.b->content.value._wcs;
  
  /* Check if member exists. */
  assert(dt_base.member_exists);
  if (!dt_base.member_exists(base, name)) {
    *error = UNE_ERROR_SET(UNE_ET_SYMBOL_NOT_DEFINED, node->content.branch.b->pos);
    une_result_free(base);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Get member. */
  assert(dt_base.get_member);
  une_result result = dt_base.get_member(base, name);
  
  /* Register base_raw as 'this' contestant. */
  une_result_free(is->this_contestant);
  is->this_contestant = base_raw;
  /* If the type of base_raw matches the type of base, we interpreted a literal. In this case, base *is* base_raw, and we should not free it. */
  if (base_raw.type != base.type)
    une_result_free(base);
  
  return result;
}

/*
Interpret a UNE_NT_CALL une_node.
*/
une_interpreter__(une_interpret_call)
{
  /* Get callable. */
  une_result callable = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (callable.type == UNE_RT_ERROR)
    return callable;
  
  /* Determine if this is a method call. */
  bool is_method_call = node->content.branch.a->type == UNE_NT_MEMBER_GET;
  une_result this_before = une_result_create(UNE_RT_VOID);
  if (is_method_call) {
    /* Protect current 'this'. */
    this_before = is->this;
    /* Promote 'this' contestant to actual 'this'. */
    assert(is->this_contestant.type == UNE_RT_GENERIC_REFERENCE || is->this_contestant.type == UNE_RT_OBJECT);
    is->this = is->this_contestant;
    is->this_contestant = une_result_create(UNE_RT_VOID);
  }
  
  /* Ensure result type is callable. */
  une_datatype dt_callable = UNE_DATATYPE_FOR_RESULT(callable);
  if (dt_callable.call == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Interpret arguments. */
  une_result args = une_result_dereference(une_interpret_list(error, is, node->content.branch.b));
  if (args.type == UNE_RT_ERROR) {
    une_result_free(callable);
    return args;
  }

  /* Execute function. */
  une_result result = dt_callable.call(error, is, node, callable, args);
  une_result_free(callable);
  une_result_free(args);
  
  /* Free our 'this' and reinstate the previous 'this'. */
  if (is_method_call) {
    une_result_free(is->this);
    is->this = this_before;
  }
  
  return result;
}

/*
Interpret a UNE_NT_FOR_RANGE une_node.
*/
une_interpreter__(une_interpret_for_range)
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
  une_association *var = une_variable_find_or_create(is->context, id); /* We only check the *local* variables. */
  
  /* Loop. */
  for (une_int i=from; i!=till; i+=step) {
    var = une_variable_find(is->context, id); /* Avoid stale pointer if variable buffer grows. */
    une_result_free(var->content);
    var->content = (une_result){
      .type = UNE_RT_INT,
      .value._int = i
    };
    result = une_result_dereference(une_interpret(error, is, node->content.branch.d));
    if (result.type == UNE_RT_ERROR || is->should_return || is->should_exit)
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
Interpret a UNE_NT_FOR_ELEMENT une_node.
*/
une_interpreter__(une_interpret_for_element)
{
  /* Get range. */
  une_result elements = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (elements.type == UNE_RT_ERROR)
    return elements;
  une_datatype elements_dt = UNE_DATATYPE_FOR_RESULT(elements);
  if (!elements_dt.get_len) {
    une_result_free(elements);
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
    return une_result_create(UNE_RT_ERROR);
  }
  une_int length = (une_int)elements_dt.get_len(elements);
  assert(elements_dt.get_index);
  
  /* Get loop variable. */
  wchar_t *id = node->content.branch.a->content.value._wcs;
  une_association *var = une_variable_find_or_create(is->context, id); /* We only check the *local* variables. */
  
  /* Prepare internal index. */
  une_result index = une_result_create(UNE_RT_INT);
  index.value._int = 0;
  
  /* Loop. */
  for (; index.value._int<length; index.value._int++) {
    var = une_variable_find(is->context, id); /* Avoid stale pointer if variable buffer grows. */
    une_result_free(var->content);
    var->content = elements_dt.get_index(elements, index);
    une_result result = une_result_dereference(une_interpret(error, is, node->content.branch.c));
    if (result.type == UNE_RT_ERROR || is->should_return || is->should_exit)
      return result;
    if (result.type == UNE_RT_BREAK) {
      une_result_free(result);
      break;
    }
    une_result_free(result);
  }
  
  une_result_free(elements);
  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_WHILE une_node.
*/
une_interpreter__(une_interpret_while)
{
  une_result result, condition;
  une_result_type result_type;
  une_int condition_is_true;

  while (true) {
    condition = une_result_dereference(une_interpret(error, is, node->content.branch.a));
    if (condition.type == UNE_RT_ERROR)
      return condition;
    condition_is_true = une_result_is_true(condition);
    une_result_free(condition);
    if (!condition_is_true)
      break;
    result = une_result_dereference(une_interpret(error, is, node->content.branch.b));
    if (result.type == UNE_RT_ERROR || is->should_return || is->should_exit)
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
une_interpreter__(une_interpret_if)
{
  /* Check if predicate applies. */
  une_result predicate = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (predicate.type == UNE_RT_ERROR)
    return predicate;
  une_int is_true = une_result_is_true(predicate);
  une_result_free(predicate);
  
  if (is_true)
    return une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (node->content.branch.c != NULL)
    return une_result_dereference(une_interpret(error, is, node->content.branch.c));
  return une_result_create(UNE_RT_VOID);
}

/*
Interpret a UNE_NT_CONTINUE une_node.
*/
une_interpreter__(une_interpret_continue)
{
  return une_result_create(UNE_RT_CONTINUE);
}

/*
Interpret a UNE_NT_BREAK une_node.
*/
une_interpreter__(une_interpret_break)
{
  return une_result_create(UNE_RT_BREAK);
}

/*
Interpret a UNE_NT_RETURN une_node.
*/
une_interpreter__(une_interpret_return)
{
  une_result result;
  if (node->content.branch.a != NULL)
    result = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  else
    result = une_result_create(UNE_RT_VOID);
  is->should_return = true;
  return result;
}

/*
Interpret a UNE_NT_EXIT une_node.
*/
une_interpreter__(une_interpret_exit)
{
  une_result result;
  if (node->content.branch.a != NULL)
    result = une_interpret_as(error, is, node->content.branch.a, UNE_RT_INT);
  else
    result = une_result_create(UNE_RT_VOID);
  is->should_exit = true;
  return result;
}

/*
Interpret a UNE_NT_COVER une_node.
*/
une_interpreter__(une_interpret_cover)
{
  /* Try to interpret branch A. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type != UNE_RT_ERROR)
    return left;
  *error = une_error_create();
  une_result_free(left);

  /* Now that we checked branch A, branch B will always hold the outcome of this function. */
  return une_result_dereference(une_interpret(error, is, node->content.branch.b));
}

/*
Interpret a UNE_NT_CONCATENATE une_node.
*/
une_interpreter__(une_interpret_concatenate)
{
  /* Evalute branches. */
  une_result left = une_result_dereference(une_interpret(error, is, node->content.branch.a));
  if (left.type == UNE_RT_ERROR)
    return left;
  une_result right = une_result_dereference(une_interpret(error, is, node->content.branch.b));
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  /* Convert both branches to strings and add them. */
  une_result concatenated;
  une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
  une_datatype dt_right = UNE_DATATYPE_FOR_RESULT(right);
  if (!dt_left.as_str) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
    concatenated = une_result_create(UNE_RT_ERROR);
  } else if (!dt_right.as_str) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
    concatenated = une_result_create(UNE_RT_ERROR);
  } else {
    une_result left_as_str = dt_left.as_str(left);
    une_result right_as_str = dt_right.as_str(right);
    concatenated = une_datatype_str_add(left_as_str, right_as_str);
    une_result_free(left_as_str);
    une_result_free(right_as_str);
  }
  
  une_result_free(left);
  une_result_free(right);
  return concatenated;
}

/*
Interpret a UNE_NT_THIS une_node.
*/
une_interpreter__(une_interpret_this)
{
  if (is->this.type == UNE_RT_GENERIC_REFERENCE)
    return is->this;
  une_result reference = une_result_create(UNE_RT_GENERIC_REFERENCE);
  reference.value._vp = (void*)&is->this;
  return reference;
}
