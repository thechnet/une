/*
interpreter.c - Une
Updated 2021-04-28
*/

#include "interpreter.h"

#pragma region une_interpret
une_result une_interpret(une_node *node, une_context *context)
{
  #ifdef UNE_DEBUG_LOG_INTERPRET
    LOGS(une_node_type_to_wcs(node->type));
  #endif
  
  switch (node->type) {

    case UNE_NT_STMTS:   return une_interpret_stmts   (node, context);
    case UNE_NT_ADD:     return une_interpret_add     (node, context);
    case UNE_NT_SUB:     return une_interpret_sub     (node, context);
    case UNE_NT_MUL:     return une_interpret_mul     (node, context);
    case UNE_NT_DIV:     return une_interpret_div     (node, context);
    case UNE_NT_FDIV:    return une_interpret_fdiv    (node, context);
    case UNE_NT_MOD:     return une_interpret_mod     (node, context);
    case UNE_NT_NEG:     return une_interpret_neg     (node, context);
    case UNE_NT_POW:     return une_interpret_pow     (node, context);
    case UNE_NT_NOT:     return une_interpret_not     (node, context);
    case UNE_NT_EQU:     return une_interpret_equ     (node, context);
    case UNE_NT_NEQ:     return une_interpret_neq     (node, context);
    case UNE_NT_GTR:     return une_interpret_gtr     (node, context);
    case UNE_NT_GEQ:     return une_interpret_geq     (node, context);
    case UNE_NT_LSS:     return une_interpret_lss     (node, context);
    case UNE_NT_LEQ:     return une_interpret_leq     (node, context);
    case UNE_NT_AND:     return une_interpret_and     (node, context);
    case UNE_NT_OR:      return une_interpret_or      (node, context);
    case UNE_NT_COP:     return une_interpret_cop     (node, context);
    case UNE_NT_IDX_GET:     return une_interpret_idx_get     (node, context);
    case UNE_NT_SET:     return une_interpret_set     (node, context);
    case UNE_NT_SET_IDX: return une_interpret_set_idx (node, context);
    case UNE_NT_GET:     return une_interpret_get     (node, context);
    case UNE_NT_FOR:     return une_interpret_for     (node, context);
    case UNE_NT_WHILE:   return une_interpret_while   (node, context);
    case UNE_NT_IF:      return une_interpret_if      (node, context);
    case UNE_NT_DEF:     return une_interpret_def     (node, context);
    case UNE_NT_CALL:    return une_interpret_call    (node, context);
    
    case UNE_NT_BREAK: {
      une_result result = une_result_create();
      result.type = UNE_RT_BREAK;
      return result; }
    
    case UNE_NT_CONTINUE: {
      une_result result = une_result_create();
      result.type = UNE_RT_CONTINUE;
      return result; }
    
    case UNE_NT_RETURN:
      context->should_return = true;
      if (node->content.branch.a == NULL)
        return une_result_create();
      return une_interpret(node->content.branch.a, context);

    case UNE_NT_LIST:
      return une_interpret_list(node, context);

    case UNE_NT_INT:
      return (une_result){
        .type = UNE_RT_INT,
        .value._int = node->content.value._int };

    case UNE_NT_FLT:
      return (une_result){
        .type = UNE_RT_FLT,
        .value._flt = node->content.value._flt };

    case UNE_NT_STR: { // DOC: Memory Management: Here we can see that results DUPLICATE strings.
      return (une_result){
        .type = UNE_RT_STR,
        .value._wcs = wcs_dup(node->content.value._wcs) }; }

    case UNE_NT_ID: {
      return (une_result){
        .type = UNE_RT_ID,
        .value._wcs = wcs_dup(node->content.value._wcs) }; }

    default:
      WERR(L"une_interpret: Unhandled node type %d", node->type);
  
  }
}
#pragma endregion une_interpret

#pragma region une_interpret_stmts
une_result une_interpret_stmts(une_node *node, une_context *context)
{
  une_result _result; /* Here it's ok to not initialize the result, as it
                         will never be returned like this. */

  UNE_UNPACK_NODE_LIST(node, nodes, nodes_size);

  UNE_FOR_NODE_LIST_ITEM(i, nodes_size) {

    _result = une_interpret(nodes[i], context);
    
    if (_result.type == UNE_RT_ERROR) {
      /* DOC: We only keep _result if there was an error. Otherwise, results of
      standalone expressions (or conditional operations, to be exact) are
      discarded right away. */
      return(_result);
    }
    
    if (
      context->should_return ||
      _result.type == UNE_RT_CONTINUE ||
      _result.type == UNE_RT_BREAK
    ) {
      return(_result);
    }
    
    /* DOC: We need to free _result after every iteration, because otherwise,
    the memory inside it is "lost" (see comment above). */
    une_result_free(_result);
  }
  
  return une_result_create();
}
#pragma endregion une_interpret_stmts

#pragma region une_interpret_add
une_result une_interpret_add(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  // INT and FLT
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
  
  // STR and STR
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR) {
    result = une_result_strs_add(left, right);
  }
  
  // LIST and LIST
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    result = une_result_lists_add(left, right);
  }

  // Error
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_ADD, node->pos,
        _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_add

#pragma region une_interpret_sub
une_result une_interpret_sub(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  // INT and FLT
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

  // Error
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_SUB, node->pos,
        _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_sub

#pragma region une_interpret_mul
une_result une_interpret_mul(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  // INT and FLT
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
  
  // INT and STR
  else if (
    (left.type == UNE_RT_INT && right.type == UNE_RT_STR) ||
    (left.type == UNE_RT_STR && right.type == UNE_RT_INT)
  ) {
    une_int count;
    une_result wcs;
    if (left.type == UNE_RT_INT) {
      count = left.value._int;
      wcs = right;
    } else {
      count = right.value._int;
      wcs = left;
    }
    if (count < 0) count = 0;
    result = une_result_str_mul(wcs, count);
  }
  
  // INT and LIST
  else if (
    (left.type == UNE_RT_INT && right.type == UNE_RT_LIST) ||
    (left.type == UNE_RT_LIST && right.type == UNE_RT_INT)
  ) {
    une_int count;
    une_result list;
    if (left.type == UNE_RT_INT) {
      count = left.value._int;
      list = right;
    } else {
      count = right.value._int;
      list = left;
    }
    if (count < 0) count = 0;
    result = une_result_list_mul(list, count);
  }

  // Error
  else {
    context->error = UNE_ERROR_SETX(
      UNE_ET_MUL, node->pos, _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_mul

#pragma region une_interpret_div
une_result une_interpret_div(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  if (
    (right.type == UNE_RT_INT && right.value._int == 0) ||
    (right.type == UNE_RT_FLT && right.value._flt == 0.0)
  ) {
    context->error = UNE_ERROR_SET(
      UNE_ET_ZERO_DIVISION, node->content.branch.b->pos);
    result.type = UNE_RT_ERROR;
  }
  
  // INT and FLT
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

  // Error
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_DIV, node->pos,
      _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_div

#pragma region une_interpret_fdiv
une_result une_interpret_fdiv(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  if (
    (right.type == UNE_RT_INT && right.value._int == 0) ||
    (right.type == UNE_RT_FLT && right.value._flt == 0.0)
  ) {
    context->error = UNE_ERROR_SET(UNE_ET_ZERO_DIVISION, 
      node->content.branch.b->pos);
    result.type = UNE_RT_ERROR;
  }
  
  // INT and FLT
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
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

  // Error
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_FDIV, node->pos,
      _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_fdiv

#pragma region une_interpret_mod
une_result une_interpret_mod(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  // INT and FLT
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

  // Error
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_MOD, node->pos,
      _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_mod

#pragma region une_interpret_neg
une_result une_interpret_neg(une_node *node, une_context *context)
{
  une_result center = une_interpret(node->content.branch.a, context);
  
  une_result result = une_result_create();
  
  if (center.type == UNE_RT_ERROR) return center;
  
  if (center.type == UNE_RT_INT) {
    result.type = UNE_RT_INT;
    result.value._int = -center.value._int;
  } else if (center.type == UNE_RT_FLT) {
    result.type = UNE_RT_FLT;
    result.value._flt = -center.value._flt;
  } else {
    context->error = UNE_ERROR_SETX(UNE_ET_NEG, node->pos,
      _int=(int)center.type, _int=0);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(center);
  
  return result;
}
#pragma endregion une_interpret_neg

#pragma region une_interpret_pow
une_result une_interpret_pow(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
// INT and FLT
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
  
// Error
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_POW, node->pos,
      _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }

  if (result.type == UNE_RT_FLT && isnan(result.value._flt)) {
    context->error = UNE_ERROR_SET(UNE_ET_UNREAL_NUMBER, node->pos);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  return result;
}
#pragma endregion une_interpret_pow

#pragma region une_interpret_not
une_result une_interpret_not(une_node *node, une_context *context)
{
  une_result center = une_interpret(node->content.branch.a, context);
  if (center.type == UNE_RT_ERROR) return center;
  
  une_result result = (une_result){
    .type = UNE_RT_INT,
    .value._int = !une_result_is_true(center)
  };
  
  une_result_free(center);
  return result;
}
#pragma endregion une_interpret_not

#pragma region une_interpret_equ
une_result une_interpret_equ(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_int is_equal = une_results_are_equal(left, right);
  
  une_result result = une_result_create();
  
  if (is_equal == -1) {
    context->error = UNE_ERROR_SETX(UNE_ET_COMPARISON, node->pos,
      _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  } else {
    result.type = UNE_RT_INT;
    result.value._int = is_equal;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_equ

#pragma region une_interpret_neq
une_result une_interpret_neq(une_node *node, une_context *context)
{
  une_result equ = une_interpret_equ(node, context);
  if (equ.type == UNE_RT_ERROR) return equ;

  equ.value._int = !equ.value._int;
  return equ;
}
#pragma endregion une_interpret_neq

#pragma region une_interpret_gtr
une_result une_interpret_gtr(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  result.type = UNE_RT_INT;
  
  // INT and FLT
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.value._int = left.value._int > right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.value._int = (une_flt)left.value._int > right.value._flt;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.value._int = left.value._flt > (une_flt)right.value._int;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.value._int = left.value._flt > right.value._flt;
  }
  
  // STR and STR
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR) {
    result.value._int = wcslen(left.value._wcs) > wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(left, left_list, left_list_size);
    UNE_UNPACK_RESULT_LIST(right, right_list, right_list_size);
    result.value._int = left_list_size > right_list_size;
  }
  
  // Illegal Comparison
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_COMPARISON, node->pos,
      _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_gtr

#pragma region une_interpret_geq
une_result une_interpret_geq(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  result.type = UNE_RT_INT;
  
  // INT and FLT
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.value._int = left.value._int >= right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.value._int = (une_flt)left.value._int >= right.value._flt;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.value._int = left.value._flt >= (une_flt)right.value._int;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.value._int = left.value._flt >= right.value._flt;
  }
  
  // STR and STR
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR) {
    result.value._int = wcslen(left.value._wcs) >= wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(left, left_list, left_list_size);
    UNE_UNPACK_RESULT_LIST(right, right_list, right_list_size);
    result.value._int = left_list_size >= right_list_size;
  }
  
  // Illegal Comparison
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_COMPARISON, node->pos,
      _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_geq

#pragma region une_interpret_lss
une_result une_interpret_lss(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  result.type = UNE_RT_INT;
  
  // INT and FLT
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.value._int = left.value._int < right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.value._int = (une_flt)left.value._int < right.value._flt;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.value._int = left.value._flt < (une_flt)right.value._int;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.value._int = left.value._flt < right.value._flt;
  }
  
  // STR and STR
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR) {
    result.value._int = wcslen(left.value._wcs) < wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(left, left_list, left_list_size);
    UNE_UNPACK_RESULT_LIST(right, right_list, right_list_size);
    result.value._int = left_list_size < right_list_size;
  }
  
  // Illegal Comparison
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_COMPARISON, node->pos,
      _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_lss

#pragma region une_interpret_leq
une_result une_interpret_leq(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  result.type = UNE_RT_INT;
  
  // INT and FLT
  if (left.type == UNE_RT_INT && right.type == UNE_RT_INT) {
    result.value._int = left.value._int <= right.value._int;
  } else if (left.type == UNE_RT_INT && right.type == UNE_RT_FLT) {
    result.value._int = (une_flt)left.value._int <= right.value._flt;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_INT) {
    result.value._int = left.value._flt <= (une_flt)right.value._int;
  } else if (left.type == UNE_RT_FLT && right.type == UNE_RT_FLT) {
    result.value._int = left.value._flt <= right.value._flt;
  }
  
  // STR and STR
  else if (left.type == UNE_RT_STR && right.type == UNE_RT_STR) {
    result.value._int = wcslen(left.value._wcs) <= wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if (left.type == UNE_RT_LIST && right.type == UNE_RT_LIST) {
    UNE_UNPACK_RESULT_LIST(left, left_list, left_list_size);
    UNE_UNPACK_RESULT_LIST(right, right_list, right_list_size);
    result.value._int = left_list_size <= right_list_size;
  }
  
  // Illegal Comparison
  else {
    context->error = UNE_ERROR_SETX(UNE_ET_COMPARISON, node->pos,
      _int=(int)left.type, _int=(int)right.type);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_leq

#pragma region une_interpret_and
une_result une_interpret_and(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = (une_result){
    .type = UNE_RT_INT,
    .value._int = une_result_is_true(left) && une_result_is_true(right)
  };

  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_and

#pragma region une_interpret_or
une_result une_interpret_or(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = (une_result){
    .type = UNE_RT_INT,
    .value._int = une_result_is_true(left) || une_result_is_true(right)
  };
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_or

#pragma region une_interpret_cop
une_result une_interpret_cop(une_node *node, une_context *context)
{
  une_result truevalue = une_interpret(node->content.branch.a, context);
  if (truevalue.type == UNE_RT_ERROR) return truevalue;
  une_result condition = une_interpret(node->content.branch.b, context);
  if (condition.type == UNE_RT_ERROR) {
    une_result_free(truevalue);
    return condition;
  }
  une_result falsevalue = une_interpret(node->content.branch.c, context);
  if (falsevalue.type == UNE_RT_ERROR) {
    une_result_free(truevalue);
    une_result_free(condition);
    return falsevalue;
  }
  
  une_result result = une_result_create();
  
  if (une_result_is_true(condition)) {
    result = truevalue;
    une_result_free(falsevalue);
  } else {
    result = falsevalue;
    une_result_free(truevalue);
  }
  
  une_result_free(condition);
  
  return result;
}
#pragma endregion une_interpret_cop

#pragma region une_interpret_idx_get
une_result une_interpret_idx_get(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create();
  
  if (right.type != UNE_RT_INT) {
    context->error = UNE_ERROR_SETX(UNE_ET_NOT_INDEX_TYPE, node->content.branch.b->pos,
        _int=(int)right.type, _int=0);
    result.type = UNE_RT_ERROR;
  } else {
    switch (left.type) {
      case UNE_RT_STR:
        if (right.value._int >= 0 && right.value._int <= wcslen(left.value._wcs)-1) {
          wchar_t *string = rmalloc(2*sizeof(*string));
          string[0] = left.value._wcs[right.value._int];
          string[1] = L'\0';
          result.type = UNE_RT_STR;
          result.value._wcs = string;
        } else {
          context->error = UNE_ERROR_SETX(UNE_ET_INDEX_OUT_OF_RANGE, node->content.branch.b->pos,
              _int=right.value._int, _int=0);
          result.type = UNE_RT_ERROR;
        }
        break;
      
      case UNE_RT_LIST: {
        une_result *list = (une_result*)left.value._vp;
        size_t list_size = list[0].value._int;
        if (right.value._int >= 0 && right.value._int <= list_size-1) {
          result = une_result_copy(list[right.value._int+1]);
        } else {
          context->error = UNE_ERROR_SETX(UNE_ET_INDEX_OUT_OF_RANGE, node->content.branch.b->pos,
              _int=right.value._int, _int=0);
          result.type = UNE_RT_ERROR;
        }
        break; }
      
      default:
        context->error = UNE_ERROR_SETX(UNE_ET_NOT_INDEXABLE, node->pos,
            _int=(int)left.type, _int=0);
        result.type = UNE_RT_ERROR;
    }
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_idx_get

#pragma region une_interpret_get
une_result une_interpret_get(une_node *node, une_context *context)
{
  une_result name = une_interpret(node->content.branch.a, context);
  if (name.type == UNE_RT_ERROR) return name;

  // Find variable in all contexts
  une_variable *var = une_variable_find_global(context, name.value._wcs);
  if (var == NULL) {
    context->error = UNE_ERROR_SETX(UNE_ET_GET, node->content.branch.a->pos,
        _wcs=wcs_dup(name.value._wcs), _int=0);
    une_result_free(name);
    return (une_result){
      .type = UNE_RT_ERROR,
    };
  }

  une_result_free(name);
  return une_result_copy(var->content);
}
#pragma endregion une_interpret_get

#pragma region une_interpret_set_idx
une_result une_interpret_set_idx(une_node *node, une_context *context)
{
// Interpret result
  une_result result = une_interpret(node->content.branch.c, context);
  if (result.type == UNE_RT_ERROR) return result;

// Get name of list
  wchar_t *name = node->content.branch.a->content.value._wcs;

// Find variable in all contexts
  une_variable *var = une_variable_find_global(context, name);
  if (var == NULL) {
    context->error = UNE_ERROR_SETX(UNE_ET_GET, node->content.branch.a->pos,
        _wcs=wcs_dup(name), _int=0);
    result.type = UNE_RT_ERROR;
    return result;
  }
  if (var->content.type != UNE_RT_LIST) {
    context->error = UNE_ERROR_SETX(UNE_ET_NOT_INDEXABLE, node->content.branch.a->pos,
        _int=(une_int)var->content.type, _int=0);
    result.type = UNE_RT_ERROR;
    return result;
  }
  une_result *list = (une_result*)var->content.value._vp;

// Get index to variable
  une_result index = une_interpret(node->content.branch.b, context);
  if (index.type == UNE_RT_ERROR) {
    une_result_free(result);
    return index;
  }
  if (index.type != UNE_RT_INT) {
    context->error = UNE_ERROR_SETX(UNE_ET_NOT_INDEX_TYPE, node->content.branch.b->pos,
        _int=(une_int)index.type, _int=0);
    result.type = UNE_RT_ERROR;
    une_result_free(result);
    une_result_free(index);
    return result;
  }
  if (index.value._int < 0 || index.value._int >= list[0].value._int) {
    context->error = UNE_ERROR_SETX(UNE_ET_INDEX_OUT_OF_RANGE,
        node->content.branch.b->pos, _int=index.value._int, _int=0);
    result.type = UNE_RT_ERROR;
    une_result_free(result); une_result_free(index);
    return result;
  }

// Set result
  list[index.value._int+1] = result;

// Cleanup
  une_result_free(result);
  une_result_free(index);
  une_result_free(list[index.value._int+1]);
  return une_result_create();
}
#pragma endregion une_interpret_set_idx

#pragma region une_interpret_set
une_result une_interpret_set(une_node *node, une_context *context)
{
  une_result result = une_interpret(node->content.branch.b, context);
  if (result.type == UNE_RT_ERROR) return result;

  wchar_t *name = node->content.branch.a->content.value._wcs;

  // Check if variable already exists *in current context*.
  une_variable *var = une_variable_find(context->variables, context->variables_count, name);

  // Create variable if it doesn't exist.
  if (var == NULL) {
    var = une_variable_create(
      &context->variables,
      &context->variables_count,
      &context->variables_size,
      name
    );
  } else {
    une_result_free(var->content);
  }
  
  var->content = result;

  return une_result_create();
}
#pragma endregion une_interpret_set

#pragma region une_interpret_for
une_result une_interpret_for(une_node *node, une_context *context) {

  wchar_t *id = node->content.branch.a->content.value._wcs;

  une_result from = une_interpret(node->content.branch.b, context);
  if (from.type == UNE_RT_ERROR) return from;
  
  une_result till = une_interpret(node->content.branch.c, context);
  if (till.type == UNE_RT_ERROR) {
    une_result_free(from);
    return till;
  }
  
  une_result result = une_result_create();
  
  if (from.type == UNE_RT_INT && till.type == UNE_RT_INT) {
    int step;
    if (from.value._int < till.value._int) {
      step = 1;
    } else {
      step = -1;
    }
    
    // Check if variable already exists *in current context*.
    une_variable *var = une_variable_find(
      context->variables, context->variables_count, id);
    if (from.value._int != till.value._int && var == NULL) {
      var = une_variable_create(
        &context->variables, &context->variables_count,
        &context->variables_size, id);
    }

    une_result _result = une_result_create();
    for (une_int i=from.value._int; i!=till.value._int; i+=step) {
      une_result_free(var->content);
      var->content.type = UNE_RT_INT;
      var->content.value._int = i;
      _result = une_interpret(node->content.branch.d, context);
      if (_result.type == UNE_RT_ERROR) {
        une_result_free(from);
        une_result_free(till);
        return _result;
      }
      if (_result.type == UNE_RT_BREAK) break;
      if (i+step != till.value._int) une_result_free(_result);
    }
    
    result = _result;
  } else {
    context->error = UNE_ERROR_SETX(UNE_ET_FOR, node->pos,
      _int=from.type != UNE_RT_INT ? (int)from.type : (int)till.type, _int=0);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(from);
  une_result_free(till);

  if (context->should_return) return result;

  une_result_free(result);
  return une_result_create();
}
#pragma endregion une_interpret_for

#pragma region une_interpret_while
une_result une_interpret_while(une_node *node, une_context *context)
{
  une_result result = une_result_create();
  une_result condition = une_result_create();

  while (true) {
    condition = une_interpret(node->content.branch.a, context);
    if (condition.type == UNE_RT_ERROR) {
      une_result_free(result);
      return condition;
    }
    if (!une_result_is_true(condition)) break;
    une_result_free(condition);
    une_result_free(result);
    result = une_interpret(node->content.branch.b, context);
    if (result.type == UNE_RT_ERROR) break;
  }

  une_result_free(condition);

  if (context->should_return) return result;

  une_result_free(result);
  return une_result_create();
}
#pragma endregion une_interpret_while

#pragma region une_interpret_if
une_result une_interpret_if(une_node *node, une_context *context)
{
  une_result condition = une_interpret(node->content.branch.a, context);
  if (condition.type == UNE_RT_ERROR) return condition;

  une_result result = une_result_create();

  if (une_result_is_true(condition)) {
    result = une_interpret(node->content.branch.b, context);
  } else if (node->content.branch.c != NULL) {
    result = une_interpret(node->content.branch.c, context);
  }

  une_result_free(condition);

  if (context->should_return) return result;

  une_result_free(result);
  return une_result_create();
}
#pragma endregion une_interpret_if

#pragma region une_interpret_def
une_result une_interpret_def(une_node *node, une_context *context)
{
  wchar_t *name = node->content.branch.a->content.value._wcs;
  // Check if function already exists *in current context*.
  une_function *fn =
    une_function_find(context->functions, context->functions_count, name);
  if (fn != NULL) {
    context->error = UNE_ERROR_SETX(UNE_ET_DEF, node->content.branch.a->pos,
        _wcs=wcs_dup(name), _int=0);
    return (une_result){.type = UNE_RT_ERROR};
  }

  UNE_UNPACK_NODE_LIST(node->content.branch.b, params_n, params_count);

  wchar_t **params = rmalloc(params_count*sizeof(*params));
  for(size_t i=0; i<params_count; i++) {
    params[i] = wcs_dup(params_n[i+1]->content.value._wcs);
  }
  une_node *body = une_node_copy(node->content.branch.c);
  fn = une_function_create(&context->functions, &context->functions_count,
    &context->functions_size, name);
  fn->params_count = params_count;
  fn->params = params;
  fn->body = body;
  return une_result_create();
}
#pragma endregion une_interpret_def

#pragma region une_interpret_call
une_result une_interpret_call(une_node *node, une_context *context)
{
  une_result result = (une_result){.type = UNE_RT_ERROR};

// Get function name
  wchar_t *name = node->content.branch.a->content.value._wcs;

// Check if function exists
  une_function *fn = une_function_find_global(context, name);
  if (fn == NULL) {
    context->error = UNE_ERROR_SETX(UNE_ET_CALL, node->content.branch.a->pos,
        _wcs=wcs_dup(name), _int=0);
    return result;
  }

// Get and verify number of arguments
  UNE_UNPACK_NODE_LIST(node->content.branch.b, args_n, args_count);
  if (args_count != fn->params_count) {
    context->error = UNE_ERROR_SETX(UNE_ET_CALL_ARGS, node->content.branch.b->pos,
        _int=fn->params_count, _int=args_count);
    return result;
  }

// Interpret arguments
  une_result *args = rmalloc(args_count*sizeof(*args));
  for (size_t i=0; i<args_count; i++) {
    args[i] = une_interpret(args_n[i+1], context);
    if (args[i].type == UNE_RT_ERROR) {
      for (size_t j=0; j<i; j++) {
        une_result_free(args[j]);
        free(args);
        return args[i];
      }
    }
  }

// Create function context
  une_context *_context = une_context_create();
  _context->parent = context;
  context = _context;
  context->name = wcs_dup(name);
  context->variables_size = UNE_SIZE_SMALL; // FIXME: SIZE
  context->variables = rmalloc(context->variables_size*sizeof(*context->variables));
  context->functions_size = UNE_SIZE_SMALL; // FIXME: SIZE
  context->functions = rmalloc(context->functions_size*sizeof(*context->functions));

// Set parameters to arguments
  for (size_t i=0; i<fn->params_count; i++) {
    une_variable *var = une_variable_create(&context->variables, &context->variables_count,
      &context->variables_size, fn->params[i]);
    var->content = une_result_copy(args[i]);
  }

// Interpret body
  result = une_interpret(fn->body, context);
  for (size_t i=0; i<args_count; i++) {
    une_result_free(args[i]);
  }
  free(args);
  if (result.type == UNE_RT_ERROR)
    context->parent->error = une_error_copy(context->error);

// Return to parent context
  _context = context->parent;
  une_context_free(context);
  context = _context;
  return result;
}
#pragma endregion une_interpret_call

#pragma region une_interpret_list
une_result une_interpret_list(une_node *node, une_context *context)
{
  UNE_UNPACK_NODE_LIST(node, list, list_size);

  une_result *result_list = rmalloc((list_size+1)*sizeof(*result_list));

  result_list[0] = (une_result){
    .type = UNE_RT_SIZE,
    .value._int = list_size,
  };

  UNE_FOR_NODE_LIST_ITEM(i, list_size) {
    result_list[i] = une_interpret(list[i], context);
    if (result_list[i].type == UNE_RT_ERROR) {
      une_result result = une_result_copy(result_list[i]);
      for (size_t j=0; j<=i; j++) {
        une_result_free(result_list[j]);
      }
      free(result_list);
      return result;
    }
  }

  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)result_list,
  };
}
#pragma endregion une_interpret_list
