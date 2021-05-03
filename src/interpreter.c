/*
interpreter.c - Une
Updated 2021-05-01
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
    case UNE_NT_GET_IDX: return une_interpret_get_idx (node, context);
    case UNE_NT_SET:     return une_interpret_set     (node, context);
    case UNE_NT_SET_IDX: return une_interpret_set_idx (node, context);
    case UNE_NT_GET:     return une_interpret_get     (node, context);
    case UNE_NT_FOR:     return une_interpret_for     (node, context);
    case UNE_NT_WHILE:   return une_interpret_while   (node, context);
    case UNE_NT_IF:      return une_interpret_if      (node, context);
    case UNE_NT_DEF:     return une_interpret_def     (node, context);
    case UNE_NT_CALL:    return une_interpret_call    (node, context);
    
    case UNE_NT_BREAK:    return une_result_create(UNE_RT_BREAK);
    case UNE_NT_CONTINUE: return une_result_create(UNE_RT_CONTINUE);
    
    case UNE_NT_RETURN:
      context->should_return = true;
      if (node->content.branch.a == NULL)
        return une_result_create(UNE_RT_VOID);
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

#pragma region une_interpret_as
static une_result une_interpret_as(une_result_type type, une_node *node, une_context *context)
{
  une_result result = une_interpret(node, context);
  if (result.type != type && result.type != UNE_RT_ERROR) {
    context->error = UNE_ERROR_SETX(
      UNE_ET_EXPECTED_RESULT_TYPE,
      node->pos,
      _int=(int)type,
      _int=0
    );
    une_result_free(result);
    result = une_result_create(UNE_RT_ERROR);
  }
  return result;
}
#pragma endregion une_interpret_as

#pragma region une_interpret_stmts
static une_result une_interpret_stmts(une_node *node, une_context *context)
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
  
  return une_result_create(UNE_RT_VOID);
}
#pragma endregion une_interpret_stmts

#pragma region une_interpret_add
static une_result une_interpret_add(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_sub(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_mul(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_div(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_fdiv(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_mod(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_neg(une_node *node, une_context *context)
{
  une_result center = une_interpret(node->content.branch.a, context);
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_pow(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_not(une_node *node, une_context *context)
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
static une_result une_interpret_equ(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_int is_equal = une_results_are_equal(left, right);
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_neq(une_node *node, une_context *context)
{
  une_result equ = une_interpret_equ(node, context);
  if (equ.type == UNE_RT_ERROR) return equ;

  equ.value._int = !equ.value._int;
  return equ;
}
#pragma endregion une_interpret_neq

#pragma region une_interpret_gtr
static une_result une_interpret_gtr(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_geq(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_lss(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_leq(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if (right.type == UNE_RT_ERROR) {
    une_result_free(left);
    return right;
  }
  
  une_result result = une_result_create(UNE_RT_VOID);
  
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
static une_result une_interpret_and(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR || !une_result_is_true(left)) return left;
  une_result_free(left);

/*
  If the second result is
  - true, we need to return it.
  - an error, we need to return it.
  - false, we still return it knowing the main
    evaluation of the expression will remain false.
*/
  return une_interpret(node->content.branch.b, context);
}
#pragma endregion une_interpret_and

#pragma region une_interpret_or
static une_result une_interpret_or(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if (left.type == UNE_RT_ERROR || une_result_is_true(left)) return left;
  une_result_free(left);

/*
  If the second result is
  - true, we need to return it.
  - an error, we need to return it.
  - false, we still return it knowing the main
    evaluation of the expression will remain false.
*/
  return une_interpret(node->content.branch.b, context);
}
#pragma endregion une_interpret_or

#pragma region une_interpret_cop
static une_result une_interpret_cop(une_node *node, une_context *context)
{
  une_result condition = une_interpret(node->content.branch.b, context);
  if (condition.type == UNE_RT_ERROR) return condition;
  
  une_int is_true = une_result_is_true(condition);
  une_result_free(condition);

  if (is_true)
    return une_interpret(node->content.branch.a, context);
  return une_interpret(node->content.branch.c, context);
}
#pragma endregion une_interpret_cop

#pragma region une_interpret_get_idx
static une_result une_interpret_get_idx(une_node *node, une_context *context)
{
// Get base.
  une_result base = une_interpret(node->content.branch.a, context);
  if (base.type == UNE_RT_ERROR) return base;

// Get index.
  une_result index_result = une_interpret_as(UNE_RT_INT, node->content.branch.b, context);
  if (index_result.type == UNE_RT_ERROR) {
    une_result_free(base);
    return index_result;
  }
  une_int index = index_result.value._int;
  une_result_free(index_result);

// Get index of base.
  une_result result;

  switch (base.type) {
    case UNE_RT_LIST:
      result = une_interpret_get_idx_list(base, index);
      if (result.type == UNE_RT_ERROR) break;
      une_result_free(base);
      return result;

    case UNE_RT_STR:
      result = une_interpret_get_idx_str(base, index);
      if (result.type == UNE_RT_ERROR) break;
      une_result_free(base);
      return result;

    // Type not indexable.
    default:
      context->error = UNE_ERROR_SETX(UNE_ET_NOT_INDEXABLE, node->pos,
        _int=(int)base.type, _int=0);
      une_result_free(base);
      return une_result_create(UNE_RT_VOID);
  }

// Index out of range (une_interpret_get_idx_TYPE returned UNE_RT_ERROR).
  une_result_free(base);
  context->error = UNE_ERROR_SETX(UNE_ET_INDEX_OUT_OF_RANGE,
    node->content.branch.b->pos, _int=index, _int=0);
  return une_result_create(UNE_RT_ERROR);
}
#pragma endregion une_interpret_get_idx

#pragma region une_interpret_get_idx_list
static une_result une_interpret_get_idx_list(une_result list, une_int index)
{
  UNE_UNPACK_RESULT_LIST(list, list_p, list_size);

  if (index < 0 || index >= list_size) return une_result_create(UNE_RT_ERROR);

  return une_result_copy(list_p[1+index]);
}
#pragma endregion une_interpret_get_idx_list

#pragma region une_interpret_get_idx_str
static une_result une_interpret_get_idx_str(une_result str, une_int index)
{
  wchar_t *string = str.value._wcs;
  size_t string_size = wcslen(string);

  if (index < 0 || index >= string_size) return une_result_create(UNE_RT_ERROR);

  wchar_t *substring = rmalloc((1+1)*sizeof(*substring));
  substring[0] = string[index];
  substring[1] = L'\0';

  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = substring
  };
}
#pragma endregion une_interpret_get_idx_str

#pragma region une_interpret_get
static une_result une_interpret_get(une_node *node, une_context *context)
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
static une_result une_interpret_set_idx(une_node *node, une_context *context)
{
// Get name of list
  wchar_t *name = node->content.branch.a->content.value._wcs;

// Find list in all contexts
  une_variable *var = une_variable_find_global(context, name);
  if (var == NULL) {
    context->error = UNE_ERROR_SETX(UNE_ET_GET, node->content.branch.a->pos,
      _wcs=wcs_dup(name), _int=0);
    return une_result_create(UNE_RT_ERROR);
  }
  if (var->content.type != UNE_RT_LIST) {
    context->error = UNE_ERROR_SETX(UNE_ET_SET_NOT_INDEXABLE, node->content.branch.a->pos,
      _int=(une_int)var->content.type, _int=0);
    return une_result_create(UNE_RT_ERROR);
  }
  UNE_UNPACK_RESULT_LIST(var->content, list, list_size);

// Get index to list
  une_result index = une_interpret_as(UNE_RT_INT, node->content.branch.b, context);
  if (index.type == UNE_RT_ERROR) return index;
  une_int index_value = index.value._int;
  une_result_free(index);
  if (index_value < 0 || index_value >= list_size) {
    context->error = UNE_ERROR_SETX(UNE_ET_INDEX_OUT_OF_RANGE,
      node->content.branch.b->pos, _int=index_value, _int=0);
    return une_result_create(UNE_RT_ERROR);
  }

// Set result
  une_result result = une_interpret(node->content.branch.c, context);
  if (result.type == UNE_RT_ERROR) return result;
  list[1+index_value] = result;

  return une_result_create(UNE_RT_VOID);
}
#pragma endregion une_interpret_set_idx

#pragma region une_interpret_set
static une_result une_interpret_set(une_node *node, une_context *context)
{
  une_result result = une_interpret(node->content.branch.b, context);
  if (result.type == UNE_RT_ERROR) return result;

  wchar_t *name = node->content.branch.a->content.value._wcs;

  // Check if variable already exists *in current context*.
  une_variable *var = une_variable_find_or_create(context, name);
  
  une_result_free(var->content);
  var->content = result;

  return une_result_create(UNE_RT_VOID);
}
#pragma endregion une_interpret_set

#pragma region une_interpret_for
static une_result une_interpret_for(une_node *node, une_context *context)
{
  une_result result = une_interpret_as(UNE_RT_INT, node->content.branch.b, context);
  if (result.type == UNE_RT_ERROR) return result;
  une_int from = result.value._int;
  une_result_free(result);

  result = une_interpret_as(UNE_RT_INT, node->content.branch.c, context);
  if (result.type == UNE_RT_ERROR) return result;
  une_int till = result.value._int;
  une_result_free(result);

  if (from == till) return une_result_create(UNE_RT_VOID);
  
  une_int step;
  if (from < till) {
    step = 1;
  } else {
    step = -1;
  }

  wchar_t *id = node->content.branch.a->content.value._wcs;

// We only check the *local* variables.
  une_variable *var = une_variable_find_or_create(context, id);

  for (une_int i=from; i!=till; i+=step) {
    une_result_free(var->content);
    var->content = (une_result){
      .type = UNE_RT_INT,
      .value._int = i
    };
    result = une_interpret(node->content.branch.d, context);
    if (result.type == UNE_RT_ERROR || context->should_return) return result;
    if (result.type == UNE_RT_BREAK) break;
    if (i+step != till) une_result_free(result);
  }
  
  une_result_free(result);
  return une_result_create(UNE_RT_VOID);
}
#pragma endregion une_interpret_for

#pragma region une_interpret_while
static une_result une_interpret_while(une_node *node, une_context *context)
{
  une_result result, condition;
  une_result_type result_type;
  une_int condition_is_true;

  while (true) {
    condition = une_interpret(node->content.branch.a, context);
    if (condition.type == UNE_RT_ERROR) return condition;
    condition_is_true = une_result_is_true(condition);
    une_result_free(condition);
    if (!condition_is_true) break;
    result = une_interpret(node->content.branch.b, context);
    if (result.type == UNE_RT_ERROR || context->should_return) return result;
    result_type = result.type;
    une_result_free(result);
    if (result_type == UNE_RT_BREAK) break;
  }

  return une_result_create(UNE_RT_VOID);
}
#pragma endregion une_interpret_while

#pragma region une_interpret_if
static une_result une_interpret_if(une_node *node, une_context *context)
{
  une_result condition = une_interpret(node->content.branch.a, context);
  if (condition.type == UNE_RT_ERROR) return condition;
  une_int is_true = une_result_is_true(condition);
  une_result_free(condition);

  une_result result = une_result_create(UNE_RT_VOID);
  if (is_true) {
    result = une_interpret(node->content.branch.b, context);
  } else if (node->content.branch.c != NULL) {
    result = une_interpret(node->content.branch.c, context);
  }

  // if (context->should_return) return result;
  // une_result_free(result);
  // return une_result_create(UNE_RT_VOID);
  return result;
}
#pragma endregion une_interpret_if

#pragma region une_interpret_def
static une_result une_interpret_def(une_node *node, une_context *context)
{
// Get function name.
  wchar_t *name = node->content.branch.a->content.value._wcs;
  
// Check if function already exists *in current context*.
  une_function *fn = une_function_find(context->functions, context->functions_count, name);
  if (fn != NULL) {
    context->error = UNE_ERROR_SETX(UNE_ET_DEF, node->content.branch.a->pos, _wcs=wcs_dup(name), _int=0);
    return une_result_create(UNE_RT_ERROR);
  }

// Copy contents.
  UNE_UNPACK_NODE_LIST(node->content.branch.b, params_n, params_count);
  wchar_t **params = rmalloc(params_count*sizeof(*params));
  for(size_t i=0; i<params_count; i++) {
    params[i] = wcs_dup(params_n[i+1]->content.value._wcs);
  }
  une_node *body = une_node_copy(node->content.branch.c);

// Define function.
  fn = une_function_create(
    &context->functions,
    &context->functions_count,
    &context->functions_size,
    name
  );
  fn->params_count = params_count;
  fn->params = params;
  fn->body = body;
  
  return une_result_create(UNE_RT_VOID);
}
#pragma endregion une_interpret_def

#pragma region une_interpret_call_def
static une_result une_interpret_call_def(une_function *fn, une_result *args, une_context *context)
{
// Create function context.
  une_context *_context = une_context_create();
  _context->parent = context;
  context = _context;
  context->name = wcs_dup(fn->name);
  context->variables_size = UNE_SIZE_SMALL; // FIXME: SIZE
  context->variables = rmalloc(context->variables_size*sizeof(*context->variables));
  context->functions_size = UNE_SIZE_SMALL; // FIXME: SIZE
  context->functions = rmalloc(context->functions_size*sizeof(*context->functions));

// Define parameters.
  for (size_t i=0; i<fn->params_count; i++) {
    une_variable *var = une_variable_create(&context->variables, &context->variables_count,
      &context->variables_size, fn->params[i]);
    var->content = une_result_copy(args[i]);
  }

// Interpret body.
  une_result result = une_interpret(fn->body, context);
  if (result.type == UNE_RT_ERROR)
    context->parent->error = une_error_copy(context->error);

// Return to parent context.
  _context = context->parent;
  une_context_free(context);
  context = _context;
  return result;
}
#pragma endregion une_interpret_call

#pragma region une_interpret_call_builtin
static une_result une_interpret_call_builtin(une_builtin_type type, une_result *args, une_context *context, une_position pos)
{
  switch (type) {
    case UNE_BIF_PRINT: return une_builtin_print(args[0], context, pos);
    case UNE_BIF_TO_INT: return une_builtin_to_int(args[0], context, pos);
    case UNE_BIF_TO_FLT: return une_builtin_to_flt(args[0], context, pos);
    case UNE_BIF_TO_STR: return une_builtin_to_str(args[0], context, pos);
    case UNE_BIF_GET_LEN: return une_builtin_get_len(args[0], context, pos);
    default: WERR(L"une_interpret_call_builtin: Unhandled builtin type %d", type);
  }
}
#pragma endregion une_interpret_call_builtin

#pragma region une_interpret_call
une_result une_interpret_call(une_node *node, une_context *context)
{
// Get function name.
  wchar_t *name = node->content.branch.a->content.value._wcs;

  size_t num_of_params;
  une_function *fn;

// Check if function is built-in.
  une_builtin_type builtin_type = une_builtin_wcs_to_type(name);
  if (builtin_type != UNE_BIF_NONE) {
    num_of_params = une_builtin_get_num_of_params(builtin_type);
  }

// Check if function exists in symbol table.
  else {
    fn = une_function_find_global(context, name);
    if (fn == NULL) {
      context->error = UNE_ERROR_SETX(UNE_ET_CALL, node->content.branch.a->pos,
        _wcs=wcs_dup(name), _int=0);
      return une_result_create(UNE_RT_ERROR);
    }
    num_of_params = fn->params_count;
  }

// Get arguments and compare number of arguments to number of parameters.
  UNE_UNPACK_NODE_LIST(node->content.branch.b, args_n, args_count);
  if (args_count != num_of_params) {
    context->error = UNE_ERROR_SETX(UNE_ET_CALL_ARGS, node->content.branch.b->pos,
      _int=num_of_params, _int=args_count);
    return une_result_create(UNE_RT_ERROR);
  }

// Interpret arguments.
  une_result *args = rmalloc(args_count*sizeof(*args));
  for (size_t i=0; i<args_count; i++) {
    args[i] = une_interpret(args_n[i+1], context);
    if (args[i].type == UNE_RT_ERROR) {
      for (size_t j=0; j<i; j++) {
        une_result_free(args[j]);
      }
      free(args);
      return args[i];
    }
  }

// Execute function.
  une_result result;
  if (builtin_type != UNE_BIF_NONE) {
    result = une_interpret_call_builtin(
      builtin_type,
      args,
      context,
      node->content.branch.b->pos
    );
  } else {
    result = une_interpret_call_def(fn, args, context);
  }
  for (size_t i=0; i<num_of_params; i++) {
    une_result_free(args[i]);
  }
  free(args);
  return result;
}
#pragma endregion une_interpret_call

#pragma region une_interpret_list
static une_result une_interpret_list(une_node *node, une_context *context)
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
