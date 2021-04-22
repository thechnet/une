/*
interpreter.c - Une
Updated 2021-04-17
*/

#include "interpreter.h"

#pragma region une_interpret
une_result une_interpret(une_node *node, une_context *context)
{
  switch(node->type)
  {
    case UNE_NT_STMTS: return une_interpret_stmts(node, context);
    
    case UNE_NT_ADD: return une_interpret_add(node, context);
    case UNE_NT_SUB: return une_interpret_sub(node, context);
    case UNE_NT_MUL: return une_interpret_mul(node, context);
    case UNE_NT_DIV: return une_interpret_div(node, context);
    case UNE_NT_FDIV: return une_interpret_fdiv(node, context);
    case UNE_NT_MOD: return une_interpret_mod(node, context);
    case UNE_NT_NEG: return une_interpret_neg(node, context);
    case UNE_NT_POW: return une_interpret_pow(node, context);
    case UNE_NT_NOT: return une_interpret_not(node, context);
    case UNE_NT_EQU: return une_interpret_equ(node, context);
    case UNE_NT_NEQ: return une_interpret_neq(node, context);
    case UNE_NT_GTR: return une_interpret_gtr(node, context);
    case UNE_NT_GEQ: return une_interpret_geq(node, context);
    case UNE_NT_LSS: return une_interpret_lss(node, context);
    case UNE_NT_LEQ: return une_interpret_leq(node, context);
    case UNE_NT_AND: return une_interpret_and(node, context);
    case UNE_NT_OR: return une_interpret_or(node, context);
    case UNE_NT_COP: return une_interpret_cop(node, context);
    case UNE_NT_IDX: return une_interpret_idx(node, context);
    
    case UNE_NT_SET: return une_interpret_set(node, context);
    case UNE_NT_GET: return une_interpret_get(node, context);
    
    case UNE_NT_FOR: return une_interpret_for(node, context);
    
    case UNE_NT_INT: {
      une_result result;
      result.type = UNE_RT_INT;
      result.value._int = node->content.value._int;
      return result; }
    
    case UNE_NT_FLT: {
      une_result result;
      result.type = UNE_RT_FLT;
      result.value._flt = node->content.value._flt;
      return result; }
    
    case UNE_NT_STR: {
      // DOC: Memory Management: Here we can see that results COPY strings.
      wchar_t *string = malloc((wcslen(node->content.value._wcs)+1)*sizeof(*string));
      if(string == NULL) WERR(L"Out of memory.");
      wcscpy(string, node->content.value._wcs);
      une_result result;
      result.type = UNE_RT_STR;
      result.value._wcs = string;
      return result; }
    
    // FIXME: Ugly, but will soon become obsolete with new node structure.
    case UNE_NT_ID: {
      wchar_t *string = malloc((wcslen(node->content.value._wcs)+1)*sizeof(*string));
      if(string == NULL) WERR(L"Out of memory.");
      wcscpy(string, node->content.value._wcs);
      une_result result;
      result.type = UNE_RT_ID;
      result.value._wcs = string;
      return result; }
    
    case UNE_NT_LIST: {
      une_node **list = (une_node**)node->content.value._vpp;
      size_t list_size = list[0]->content.value._int;
      une_result *new_list = malloc((list_size+1)*sizeof(*new_list));
      if(new_list == NULL) WERR(L"Out of memory.");
      new_list[0].type = UNE_RT_SIZE;
      new_list[0].value._int = list_size;
      for(size_t i=1; i<=list_size; i++)
      {
        new_list[i] = une_interpret(list[i], context);
      }
      une_result result;
      result.type = UNE_RT_LIST;
      result.value._vp = new_list;
      return result; }
    
    default: WERR(L"une_interpret: Node type not implemented");
  }
}
#pragma endregion une_interpret

#pragma region une_interpret_stmts
une_result une_interpret_stmts(une_node *node, une_context *context)
{
  // Default Return Value of a Block or Program
  une_result result;
  result.type = UNE_RT_INT;
  result.value._int = 46;
  // result.type = UNE_RT_STR;
  // result.value._wcs = malloc(100*sizeof(wchar_t));
  // wcscpy(result.value._wcs, L"test");
  
  une_node **nodes = (une_node**)node->content.value._vpp;
  size_t nodes_size = nodes[0]->content.value._int;
  for(size_t i=1; i<=nodes_size; i++)
  {
    if(nodes[i]->type == UNE_NT_CONTINUE)
    {
      une_result_free(result);
      result.type = UNE_RT_CONTINUE;
      break;
    }
    if(nodes[i]->type == UNE_NT_BREAK)
    {
      une_result_free(result);
      result.type = UNE_RT_BREAK;
      break;
    }
    if(nodes[i]->type == UNE_NT_RETURN)
    {
      // DOC: If this case applies, it means the program defines its own result,
      // meaning we should free the default result.
      if(nodes[i]->content.branch.a != NULL)
      {
        une_result_free(result);
        result = une_interpret(nodes[i]->content.branch.a, context);
      }
      // Since there is a break stmt following here no matter what, we don't
      // need to check whether une_interpret returned an error or not,
      // since we are going to return the result right away either way.
      break;
    }
    une_result _result = une_interpret(nodes[i], context); // FIXME: Check this.
    if(_result.type == UNE_RT_ERROR)
    {
      // DOC: We only keep _result if there was an error. Otherwise, results of
      // standalone expressions (or conditional operations, to be exact) are discarded right away.
      une_result_free(result);
      result = _result;
      break;
    }
    // DOC: We need to free _result after every iteration because its contents
    // are not stored anywhere. (See comment above.)
    une_result_free(_result);
  }
  
  return result;
}
#pragma endregion une_interpret_stmts

#pragma region une_interpret_add
une_result une_interpret_add(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int + right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = (float)left.value._int + right.value._flt;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt + (float)right.value._int;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt + right.value._flt;
  }
  
  // STR and STR
  else if(left.type == UNE_RT_STR && right.type == UNE_RT_STR)
  {
    size_t left_len = wcslen(left.value._wcs);
    size_t right_len = wcslen(right.value._wcs);
    wchar_t *wcs = malloc((left_len+right_len+1)*sizeof(*wcs));
    if(wcs == NULL) WERR(L"Out of memory.");
    wcscpy(wcs, left.value._wcs);
    wcscpy(wcs+left_len, right.value._wcs);
    result.type = UNE_RT_STR;
    result.value._wcs = wcs;
  }
  
  // LIST and LIST
  else if(left.type == UNE_RT_LIST && right.type == UNE_RT_LIST)
  {
    // FIXME: Unsafe, doesn't duplicate strings etc., only pointers.
    une_result *left_list = (une_result*)left.value._vp;
    une_result *right_list = (une_result*)right.value._vp;
    size_t left_size = left_list[0].value._int;
    size_t right_size = right_list[0].value._int;
    une_result *new_list = malloc((left_size+right_size+1)*sizeof(*new_list));
    if(new_list == NULL) WERR(L"Out of memory.");
    new_list[0].type = UNE_RT_SIZE;
    new_list[0].value._int = left_size + right_size;
    memcpy((void*)(new_list+1),
           (void*)(left_list+1),
           left_size*sizeof(une_result));
    memcpy((void*)(new_list+1+left_size),
           (void*)(right_list+1),
           right_size*sizeof(une_result));
    result.type = UNE_RT_LIST;
    result.value._vp = (void*)new_list;
  }

  // Error
  else
  {
    context->error.type = UNE_ET_ADD;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int - right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = (float)left.value._int - right.value._flt;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt - (float)right.value._int;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt - right.value._flt;
  }

  // Error
  else
  {
    context->error.type = UNE_ET_SUB;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int *right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = (float)left.value._int *right.value._flt;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt *(float)right.value._int;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt *right.value._flt;
  }
  
  // INT and STR
  else if((left.type == UNE_RT_INT && right.type == UNE_RT_STR)
       || (left.type == UNE_RT_STR && right.type == UNE_RT_INT))
  {
    int count;
    wchar_t *wcs;
    if(left.type == UNE_RT_INT)
    {
      count = left.value._int;
      wcs = right.value._wcs;
    }
    else
    {
      count = right.value._int;
      wcs = left.value._wcs;
    }
    if(count < 0) count = 0;
    size_t wcs_size = wcslen(wcs);
    wchar_t *string = malloc((wcs_size*count+1)*sizeof(*string));
    if(string == NULL) WERR(L"Out of memory.");
    for(int i=0; i<count; i++)
    {
      wcscpy(string+i*wcs_size, wcs);
    }
    result.type = UNE_RT_STR;
    result.value._wcs = string;
  }
  
  // INT and LIST
  else if((left.type == UNE_RT_INT && right.type == UNE_RT_LIST)
       || (left.type == UNE_RT_LIST && right.type == UNE_RT_INT))
  {
    int count;
    une_result *list;
    if(left.type == UNE_RT_INT)
    {
      count = left.value._int;
      list = (une_result*)right.value._vp;
    }
    size_t list_size = list[0].value._int;
    une_result *new_list = malloc(count*list_size*sizeof(*new_list));
    if(new_list == NULL) WERR(L"Out of memory.");
    new_list[0].type = UNE_RT_ERROR;
    new_list[0].value._int = list_size;
    for(int i=0; i<count; i++)
    {
      memcpy((void*)(new_list+1+i*count),
             (void*)(list+1),
             list_size*sizeof(une_result));
    }
    une_result_free(left);
    une_result_free(right);
    result.type = UNE_RT_LIST;
    result.value._vp = (void*)new_list;
  }

  // Error
  else
  {
    context->error.type = UNE_ET_MUL;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  if((right.type == UNE_RT_INT && right.value._int == 0)
  || (right.type == UNE_RT_FLT && right.value._flt == 0.0))
  {
    context->error.type = UNE_ET_ZERO_DIVISION;
    context->error.pos = node->content.branch.b->pos;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    result.type = UNE_RT_ERROR;
  }
  
  // INT and FLT
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    if(left.value._int % right.value._int == 0)
    {
      result.type = UNE_RT_INT;
      result.value._int = left.value._int / right.value._int;
    }
    else
    {
      result.type = UNE_RT_FLT;
      result.value._flt = (float)left.value._int / right.value._int;
    }
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = (float)left.value._int / right.value._flt;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt / (float)right.value._int;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = left.value._flt / right.value._flt;
  }

  // Error
  else
  {
    context->error.type = UNE_ET_DIV;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  if((right.type == UNE_RT_INT && right.value._int == 0)
  || (right.type == UNE_RT_FLT && right.value._flt == 0.0))
  {
    context->error.type = UNE_ET_ZERO_DIVISION;
    context->error.pos = node->content.branch.b->pos;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    result.type = UNE_RT_ERROR;
  }
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int / right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = floor((float)left.value._int / right.value._flt);
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = floor(left.value._flt / (float)right.value._int);
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = floor(left.value._flt / right.value._flt);
  }

  // Error
  else
  {
    context->error.type = UNE_ET_FDIV;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_INT;
    result.value._int = left.value._int % right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = fmod((float)left.value._int, right.value._flt);
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = fmod(left.value._flt, (float)right.value._int);
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = fmod(left.value._flt, right.value._flt);
  }

  // Error
  else
  {
    context->error.type = UNE_ET_MOD;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  
  une_result result;
  
  if(center.type == UNE_RT_ERROR) return center;
  
  if(center.type == UNE_RT_INT)
  {
    result.type = UNE_RT_INT;
    result.value._int = -center.value._int;
  }
  else if(center.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = -center.value._flt;
  }
  else
  {
    context->error.type = UNE_ET_NEG;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)center.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_INT;
    result.value._int = pow(left.value._int, right.value._int);
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = powf((float)left.value._int, right.value._flt);
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = powf(left.value._flt, (float)right.value._int);
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.type = UNE_RT_FLT;
    result.value._flt = powf(left.value._flt, right.value._flt);
  }
  
  // Error
  else
  {
    context->error.type = UNE_ET_POW;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(center.type == UNE_RT_ERROR) return center;
  
  une_result result;
  
  result.type = UNE_RT_INT;
  result.value._int = !une_result_is_true(center);
  
  une_result_free(center);
  
  return result;
}
#pragma endregion une_interpret_not

#pragma region une_interpret_equ
une_result une_interpret_equ(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  une_int is_equal = une_results_are_equal(left, right);
  
  if(is_equal == -1)
  {
    context->error.type = UNE_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    result.type = UNE_RT_ERROR;
  }
  else
  {
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
  une_result left = une_interpret(node->content.branch.a, context);
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  une_int is_equal = une_results_are_equal(left, right);
  
  if(is_equal == -1)
  {
    context->error.type = UNE_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    result.type = UNE_RT_ERROR;
  }
  else
  {
    result.type = UNE_RT_INT;
    result.value._int = !is_equal;
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_neq

#pragma region une_interpret_gtr
une_result une_interpret_gtr(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  result.type = UNE_RT_INT;
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.value._int = left.value._int > right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.value._int = (float)left.value._int > right.value._flt;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.value._int = left.value._flt > (float)right.value._int;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.value._int = left.value._flt > right.value._flt;
  }
  
  // STR and STR
  else if(left.type == UNE_RT_STR && right.type == UNE_RT_STR)
  {
    result.value._int = wcslen(left.value._wcs) > wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if(left.type == UNE_RT_LIST && right.type == UNE_RT_LIST)
  {
    result.value._int =
      ((une_result*)left.value._vp)[0].value._int > ((une_result*)right.value._vp)[0].value._int;
  }
  
  // Illegal Comparison
  else
  {
    context->error.type = UNE_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  result.type = UNE_RT_INT;
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.value._int = left.value._int >= right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.value._int = (float)left.value._int >= right.value._flt;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.value._int = left.value._flt >= (float)right.value._int;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.value._int = left.value._flt >= right.value._flt;
  }
  
  // STR and STR
  else if(left.type == UNE_RT_STR && right.type == UNE_RT_STR)
  {
    result.value._int = wcslen(left.value._wcs) >= wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if(left.type == UNE_RT_LIST && right.type == UNE_RT_LIST)
  {
    result.value._int =
      ((une_result*)left.value._vp)[0].value._int >= ((une_result*)right.value._vp)[0].value._int;
  }
  
  // Illegal Comparison
  else
  {
    context->error.type = UNE_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  result.type = UNE_RT_INT;
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.value._int = left.value._int < right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.value._int = (float)left.value._int < right.value._flt;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.value._int = left.value._flt < (float)right.value._int;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.value._int = left.value._flt < right.value._flt;
  }
  
  // STR and STR
  else if(left.type == UNE_RT_STR && right.type == UNE_RT_STR)
  {
    result.value._int = wcslen(left.value._wcs) < wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if(left.type == UNE_RT_LIST && right.type == UNE_RT_LIST)
  {
    result.value._int =
      ((une_result*)left.value._vp)[0].value._int < ((une_result*)right.value._vp)[0].value._int;
  }
  
  // Illegal Comparison
  else
  {
    context->error.type = UNE_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  result.type = UNE_RT_INT;
  
  // INT and FLT
  if(left.type == UNE_RT_INT && right.type == UNE_RT_INT)
  {
    result.value._int = left.value._int <= right.value._int;
  }
  else if(left.type == UNE_RT_INT && right.type == UNE_RT_FLT)
  {
    result.value._int = (float)left.value._int <= right.value._flt;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_INT)
  {
    result.value._int = left.value._flt <= (float)right.value._int;
  }
  else if(left.type == UNE_RT_FLT && right.type == UNE_RT_FLT)
  {
    result.value._int = left.value._flt <= right.value._flt;
  }
  
  // STR and STR
  else if(left.type == UNE_RT_STR && right.type == UNE_RT_STR)
  {
    result.value._int = wcslen(left.value._wcs) <= wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if(left.type == UNE_RT_LIST && right.type == UNE_RT_LIST)
  {
    result.value._int =
      ((une_result*)left.value._vp)[0].value._int <= ((une_result*)right.value._vp)[0].value._int;
  }
  
  // Illegal Comparison
  else
  {
    context->error.type = UNE_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
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
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  result.type = UNE_RT_INT;
  result.value._int = une_result_is_true(left) && une_result_is_true(right);
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_and

#pragma region une_interpret_or
une_result une_interpret_or(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  result.type = UNE_RT_INT;
  result.value._int = une_result_is_true(left) || une_result_is_true(right);
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_or

#pragma region une_interpret_cop
une_result une_interpret_cop(une_node *node, une_context *context)
{
  une_result truevalue = une_interpret(node->content.branch.a, context);
  if(truevalue.type == UNE_RT_ERROR) return truevalue;
  une_result condition = une_interpret(node->content.branch.b, context);
  if(condition.type == UNE_RT_ERROR)
  {
    une_result_free(truevalue);
    return condition;
  }
  une_result falsevalue = une_interpret(node->content.branch.c, context);
  if(falsevalue.type == UNE_RT_ERROR)
  {
    une_result_free(truevalue);
    une_result_free(condition);
    return falsevalue;
  }
  
  une_result result;
  
  if(une_result_is_true(condition))
  {
    result = truevalue;
    une_result_free(falsevalue);
  }
  else
  {
    result = falsevalue;
    une_result_free(truevalue);
  }
  
  une_result_free(condition);
  
  return result;
}
#pragma endregion une_interpret_cop

#pragma region une_interpret_idx
une_result une_interpret_idx(une_node *node, une_context *context)
{
  une_result left = une_interpret(node->content.branch.a, context);
  if(left.type == UNE_RT_ERROR) return left;
  une_result right = une_interpret(node->content.branch.b, context);
  if(right.type == UNE_RT_ERROR)
  {
    une_result_free(left);
    return right;
  }
  
  une_result result;
  
  if(right.type != UNE_RT_INT)
  {
    context->error.type = UNE_ET_NOT_INDEX_TYPE;
    context->error.pos = node->content.branch.b->pos;
    context->error.values[0]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    result.type = UNE_RT_ERROR;
  }
  else
  {
    switch(left.type)
    {
      case UNE_RT_STR:
        if(right.value._int >= 0 && right.value._int <= wcslen(left.value._wcs)-1)
        {
          wchar_t *string = malloc(2*sizeof(*string));
          if(string == NULL) WERR(L"Out of memory.");
          string[0] = left.value._wcs[right.value._int];
          string[1] = L'\0';
          result.type = UNE_RT_STR;
          result.value._wcs = string;
        }
        else
        {
          context->error.type = UNE_ET_INDEX_OUT_OF_RANGE;
          context->error.pos = node->content.branch.b->pos;
          context->error.values[0]._int = right.value._int;
          context->error.__line__ = __LINE__;
          strcpy(context->error.__file__, __FILE__);
          result.type = UNE_RT_ERROR;
        }
        break;
      
      case UNE_RT_LIST: {
        une_result *list = (une_result*)left.value._vp;
        size_t list_size = list[0].value._int;
        if(right.value._int >= 0 && right.value._int <= list_size-1)
        {
          result = une_result_copy(list[right.value._int+1]);
        }
        else
        {
          context->error.type = UNE_ET_INDEX_OUT_OF_RANGE;
          context->error.pos = node->content.branch.b->pos;
          context->error.values[0]._int = right.value._int;
          context->error.__line__ = __LINE__;
          strcpy(context->error.__file__, __FILE__);
          result.type = UNE_RT_ERROR;
        }
        break; }
      
      default:
        context->error.type = UNE_ET_NOT_INDEXABLE;
        context->error.pos = node->pos;
        context->error.values[0]._int = (int)left.type;
        context->error.__line__ = __LINE__;
        strcpy(context->error.__file__, __FILE__);
        result.type = UNE_RT_ERROR;
    }
  }
  
  une_result_free(left);
  une_result_free(right);
  
  return result;
}
#pragma endregion une_interpret_idx

#pragma region une_interpret_set
une_result une_interpret_set(une_node *node, une_context *context)
{
  une_result name = une_interpret(node->content.branch.a, context);
  if(name.type == UNE_RT_ERROR) return name;
  une_result value = une_interpret(node->content.branch.b, context);
  if(value.type == UNE_RT_ERROR)
  {
    une_result_free(name);
    return value;
  }
  
  une_result result;
  
  if(name.type != UNE_RT_ID)
  {
    context->error.type = UNE_ET_SET;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)name.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    result.type = UNE_RT_ERROR;
    
    // Here we are freeing 'value' because we no longer want it.
    une_result_free(value);
  }
  else
  {
    result = value;
    
    // Check if variable already exists.
    int exists_at = -1;
    for(size_t i=0; i<context->variables_count; i++)
    {
      if(wcscmp(context->variables[i].name, name.value._wcs) == 0)
      {
        exists_at = i;
        break;
      }
    }
    
    // Variable does not exist
    if(exists_at == -1)
    {
      une_variable var;
      var.name = malloc((wcslen(name.value._wcs)+1)*sizeof(*var.name));
      if(var.name == NULL) WERR(L"Out of memory.");
      wcscpy(var.name, name.value._wcs);
      var.content = une_result_copy(value);
      if(context->variables_count >= context->variables_size)
      {
        context->variables_size *= 2;
        une_variable *_variables = realloc(context->variables, context->variables_size*sizeof(*_variables));
        if(_variables == NULL) WERR(L"Out of memory.");
        context->variables = _variables;
      }
      context->variables[context->variables_count] = var;
      context->variables_count++;
    }
    
    // Variable does exist
    else
    {
      une_result_free(context->variables[exists_at].content);
      context->variables[exists_at].content = une_result_copy(value);
    }

    // Here we are NOT freeing 'value' because we can use it as return value.
  }
  
  une_result_free(name);
  
  return result;
}
#pragma endregion une_interpret_set

#pragma region une_interpret_get
une_result une_interpret_get(une_node *node, une_context *context)
{
  une_result name = une_interpret(node->content.branch.a, context);
  if(name.type == UNE_RT_ERROR) return name;
  
  // We don't need to check if name.type is UNE_RT_ID because the parser
  // _only_ creates a UNE_NT_GET if it finds an UNE_TT_ID.
  
  une_result result;
  
  // Check if variable exists.
  int exists_at = -1;
  for(size_t i=0; i<context->variables_count; i++)
  {
    if(wcscmp(context->variables[i].name, name.value._wcs) == 0)
    {
      exists_at = i;
      break;
    }
  }
  
  // Variable exists.
  if(exists_at > -1)
  {
    result = une_result_copy(context->variables[exists_at].content);
    
    // Here we free 'name' because we no longer want it.
    une_result_free(name);
  }
  
  // Variable does not exist.
  else
  {
    context->error.type = UNE_ET_GET;
    context->error.pos = node->pos;
    context->error.values[0]._wcs = name.value._wcs;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    result.type = UNE_RT_ERROR;
    
    // Here we DON'T free 'name' because we can use its WCS in our return value.
  }
  
  return result;
}
#pragma endregion une_interpret_get

#pragma region une_interpret_for
une_result une_interpret_for(une_node *node, une_context *context)
{
  une_result id = une_interpret(node->content.branch.a, context);
  if(id.type == UNE_RT_ERROR) return id; // FIXME: Unnecessary?
  
  // We don't need to check if name.type is UNE_RT_ID because the parser
  // _only_ creates a UNE_NT_GET if it finds an UNE_TT_ID.
  
  une_result from = une_interpret(node->content.branch.b, context);
  if(from.type == UNE_RT_ERROR)
  {
    une_result_free(id);
    return from;
  }
  
  une_result to = une_interpret(node->content.branch.c, context);
  if(to.type == UNE_RT_ERROR)
  {
    une_result_free(id);
    une_result_free(from);
    return to;
  }
  
  une_result result;
  
  if(from.type == UNE_RT_INT && to.type == UNE_RT_INT)
  {
    int step;
    if(from.value._int < to.value._int)
    {
      step = 1;
    }
    else
    {
      step = -1;
    }
    
    une_result _result;
    for(une_int i=from.value._int; i!=to.value._int; i+=step)
    {
      _result = une_interpret(node->content.branch.d, context);
      if(_result.type == UNE_RT_ERROR)
      {
        une_result_free(id);
        une_result_free(from);
        une_result_free(to);
        return _result;
      }
      if(_result.type == UNE_RT_BREAK) break;
      if(i+step != to.value._int) une_result_free(_result);
    }
    
    result = _result;
  }
  else
  {
    context->error.type = UNE_ET_FOR;
    context->error.pos = node->pos;
    context->error.values[0]._int =
      from.type != UNE_RT_INT ? (int)from.type : (int)to.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    result.type = UNE_RT_ERROR;
  }
  
  une_result_free(id);
  une_result_free(from);
  une_result_free(to);
  
  return result;
}
#pragma endregion une_interpret_for