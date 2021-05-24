/*
parser.c - Une
Updated 2021-05-24
*/

#include "parser.h"

#pragma region une_parse
une_node *une_parse(une_instance *inst)
{
  inst->ps.index = 0; // FIXME: Necessary? (initially added because of lexer.c working with this value)
  
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:parse [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  return une_parse_sequence(
    inst,
    UNE_NT_STMTS,
    __UNE_TT_none__, UNE_TT_NEW, UNE_TT_EOF,
    &une_parse_stmt
  );
}
#pragma endregion une_parse

#pragma region une_p_peek
static une_token une_p_peek(une_instance *inst)
{
  return inst->ps.tokens[inst->ps.index];
}
#pragma endregion une_p_peek

#pragma region une_p_consume
static void une_p_consume(une_instance *inst)
{
  inst->ps.index++;
}
#pragma endregion une_p_consume

#pragma region une_p_index_get
static size_t une_p_index_get(une_instance *inst)
{
  return inst->ps.index;
}
#pragma endregion une_p_index_get

#pragma region une_p_index_set
static void une_p_index_set(une_instance *inst, size_t index)
{
  inst->ps.index = index;
}
#pragma endregion une_p_index_set

#pragma region une_parse_sequence
static une_node *une_parse_sequence(
  une_instance *inst,
  une_node_type node_type,
  une_token_type tt_begin, une_token_type tt_end_of_item, une_token_type tt_end,
  une_node* (*parser)(une_instance*)
)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:sequence [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  // [Begin Sequence]
  if (tt_begin != __UNE_TT_none__) {
    if (une_p_peek(inst).type != tt_begin) {
      inst->error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, une_p_peek(inst).pos,
                                   _int=(int)tt_begin, _int=0);
      return NULL;
    }
    une_p_consume(inst);
  }
  
  // [Sequence]
  size_t pos_start = une_p_peek(inst).pos.start;
  size_t sequence_size = UNE_SIZE_MEDIUM;
  une_node **sequence = rmalloc(sequence_size*sizeof(*sequence));
  size_t sequence_index = 1;

  while (true) {
    if (sequence_index >= sequence_size) {
      sequence_size *= 2;
      une_node **_sequence = rrealloc(sequence, sequence_size*sizeof(_sequence));
      sequence = _sequence;
    }
    
    // ADDITIONAL WHITESPACE
    while (
      une_p_peek(inst).type == tt_end_of_item ||
      une_p_peek(inst).type == UNE_TT_NEW
    ) une_p_consume(inst);

    // EXPECTED END OF SEQUENCE
    if (une_p_peek(inst).type == tt_end) break;

    // UNEXPECTED END OF SEQUENCE
    if (une_p_peek(inst).type == UNE_TT_EOF) {
      /* This can happen if a block, list, list of parameters, or list of
      arguments is opened at the end of the file without being closed. */
      for (size_t i=1; i<sequence_index; i++) une_node_free(sequence[i], false);
      free(sequence);
      inst->error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_TOKEN, une_p_peek(inst).pos,
                                   _int=(int)UNE_TT_EOF, _int=0);
      return NULL;
    }

    // PARSE ITEM
    sequence[sequence_index] = (*parser)(inst);
    if (sequence[sequence_index] == NULL) {
      for (size_t i=1; i<sequence_index; i++) une_node_free(sequence[i], false);
      free(sequence);
      return NULL;
    }
    sequence_index++;
    
    // ITEM DELIMITER
    if (
      une_p_peek(inst).type == tt_end ||
      une_p_peek(inst).type == tt_end_of_item ||
      une_p_peek(inst).type == UNE_TT_NEW
    ) continue;
    for (size_t i=1; i<sequence_index; i++) une_node_free(sequence[i], false);
    free(sequence);
    inst->error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, une_p_peek(inst).pos,
                            _int=(int)tt_end_of_item, _int=(int)tt_end);
    return NULL;
  }

  // CREATE NODE
  une_node *counter = une_node_create(UNE_NT_SIZE);
  counter->content.value._int = sequence_index-1;
  sequence[0] = counter;
  une_node *node = une_node_create(node_type);
  node->pos = (une_position){
    .start = pos_start,
    .end = une_p_peek(inst).pos.end
  };
  node->content.value._vpp = (void**)sequence;
  
  // [End Sequence]
  /* We don't skip EOF because it may still be needed by other functions up
  the call chain. */
  if (tt_end != UNE_TT_EOF) une_p_consume(inst);
  return node;
}
#pragma endregion une_parse_sequence

#pragma region une_parse_stmt
static une_node *une_parse_stmt(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:stmt [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  if (une_p_peek(inst).type == UNE_TT_NEW) une_p_consume(inst);
  
  switch (une_p_peek(inst).type) {
    case UNE_TT_FOR: return une_parse_for(inst);
    case UNE_TT_IF: return une_parse_if(inst);
    case UNE_TT_WHILE: return une_parse_while(inst);
    case UNE_TT_DEF: return une_parse_def(inst);
    case UNE_TT_RETURN: return une_parse_return(inst);
    
    #pragma region Block
    case UNE_TT_LBRC: return une_parse_sequence(
      inst,
      UNE_NT_STMTS,
      UNE_TT_LBRC, UNE_TT_NEW, UNE_TT_RBRC,
      &une_parse_stmt
    );
    #pragma endregion Block
    
    #pragma region Break
    case UNE_TT_BREAK: {
      if (!inst->ps.inside_loop) {
        inst->error = UNE_ERROR_SET(UNE_ET_BREAK_OUTSIDE_LOOP, une_p_peek(inst).pos);
        return NULL;
      }
      une_node *break_ = une_node_create(UNE_NT_BREAK);
      break_->pos = une_p_peek(inst).pos;
      une_p_consume(inst);
      return break_;
    }
    #pragma endregion Break
    
    #pragma region Continue
    case UNE_TT_CONTINUE: {
      if (!inst->ps.inside_loop) {
        inst->error = UNE_ERROR_SET(UNE_ET_CONTINUE_OUTSIDE_LOOP, une_p_peek(inst).pos);
        return NULL;
      }
      une_node *continue_ = une_node_create(UNE_NT_CONTINUE);
      continue_->pos = une_p_peek(inst).pos;
      une_p_consume(inst);
      return continue_;
    }
    #pragma endregion Continue
    
    #pragma region Variable Definition
    case UNE_TT_ID: {
      // [Id]
      size_t token_index_before = une_p_index_get(inst); // Needed to backstep in case this is not a variable definition.
      
      une_node *target = une_parse_id(inst);
      if (target == NULL) return NULL;
      
      une_node *varset = une_node_create(UNE_NT_SET);
      varset->pos.start = target->pos.start;
      varset->content.branch.a = target;
      
      // [Index]
      if (une_p_peek(inst).type == UNE_TT_LSQB) {
        varset->type = UNE_NT_SET_IDX;
        une_p_consume(inst);
        
        une_node *position = une_parse_expression(inst);
        if (position == NULL) {
          une_node_free(varset, false);
          return NULL;
        }
        varset->content.branch.b = position;
        
        if (une_p_peek(inst).type != UNE_TT_RSQB) {
          une_node_free(varset, false);
          return NULL;
        }
        une_p_consume(inst);
      }
      
      // [Expression]
      if (une_p_peek(inst).type == UNE_TT_SET) {
        une_p_consume(inst);
        
        une_node *expression = une_parse_expression(inst);
        if (expression == NULL) {
          une_node_free(varset, false);
          return NULL;
        }
        
        varset->pos.end = expression->pos.end;
        if (varset->type == UNE_NT_SET_IDX) {
          varset->content.branch.c = expression;
        } else {
          varset->content.branch.b = expression;
        }
        return varset;
      }
      
      // Not a Variable Definition - Fall through to expression.
      une_node_free(varset, false);
      une_p_index_set(inst, token_index_before);
      /*
      Notice how there is no break here: If the above code ended here it means
      the stmt is not a variable definition, leaving the only option
      for it to be a conditional operation.
      */
    }
    #pragma endregion Variable Definition

    #pragma region Expression
    default: return une_parse_expression(inst);
    #pragma endregion Expression
  }
}
#pragma endregion une_parse_stmt

#pragma region une_parse_if
static une_node *une_parse_if(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:if [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  size_t pos_start = une_p_peek(inst).pos.start;
  
  // [If || Elif]
  une_p_consume(inst);

  // [Condition]
  une_node *condition = une_parse_expression(inst);
  if (condition == NULL) return NULL;
  
  // [Body]
  une_node *truebody = une_parse_stmt(inst);
  if (truebody == NULL) {
    une_node_free(condition, false);
    return NULL;
  }

  /*
  Whitespace. This counts for both 'elif' and 'else' because 'elif'
  creates an entirely new if node where this stmt then removes
  whitespace in front of 'else'.
  */
  size_t _token_index = une_p_index_get(inst); /* Here we skip over whitespace expecting
                                                  an elif or else clause. If we don't
                                                  find one, however, we have now skipped
                                                  the whitespace that tells
                                                  une_parse_sequence that a new command
                                                  is starting. Therefore, we need to
                                                  return back to this index in case there
                                                  is no clause following the if clause.
                                                  */
  if (une_p_peek(inst).type == UNE_TT_NEW) une_p_consume(inst);

  une_node *ifstmt = une_node_create(UNE_NT_IF);
  ifstmt->pos.start = pos_start;
  ifstmt->content.branch.a = condition;
  ifstmt->content.branch.b = truebody;

  // Only If Body
  if (
    une_p_peek(inst).type != UNE_TT_ELSE &&
    une_p_peek(inst).type != UNE_TT_ELIF
  ) {
    une_p_index_set(inst, _token_index);
    ifstmt->pos.end = truebody->pos.end;
    return ifstmt;
  }

  // FIXME: ↑ ↓ Is there a smarter way to do this?
  
  // [Elif Body || Else Body]
  une_node *falsebody;
  if (une_p_peek(inst).type == UNE_TT_ELIF) {
    falsebody = une_parse_if(inst);
  } else {
    une_p_consume(inst);
    falsebody = une_parse_stmt(inst);
  }
  if (falsebody == NULL) {
    une_node_free(ifstmt, false);
    return NULL;
  }
  ifstmt->pos.end = falsebody->pos.end;
  ifstmt->content.branch.c = falsebody;
  return ifstmt;
}
#pragma endregion une_parse_if

#pragma region une_parse_for
static une_node *une_parse_for(une_instance *inst) {
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:for [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  size_t pos_start = une_p_peek(inst).pos.start;
  
  // [For]
  une_p_consume(inst);
  
  // [Id]
  une_node *counter = une_parse_id(inst);
  if (counter == NULL) return NULL;
  
  // [From]
  if (une_p_peek(inst).type != UNE_TT_FROM) {
    inst->error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, une_p_peek(inst).pos,
                            _int=UNE_TT_FROM, _int=0);
    une_node_free(counter, false);
    return NULL;
  }
  une_p_consume(inst);
  
  // [Expression]
  une_node *from = une_parse_expression(inst);
  if (from == NULL) {
    une_node_free(counter, false);
    return NULL;
  }
  
  // [To]
  if (une_p_peek(inst).type != UNE_TT_TILL) {
    inst->error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, une_p_peek(inst).pos,
                            _int=UNE_TT_TILL, _int=0);
    une_node_free(counter, false);
    une_node_free(from, false);
    return NULL;
  }
  une_p_consume(inst);
  
  // [Expression]
  une_node *to = une_parse_expression(inst);
  if (to == NULL) {
    une_node_free(counter, false);
    une_node_free(from, false);
    return NULL;
  }
  
  // [Body]
  inst->ps.inside_loop = true;
  une_node *body = une_parse_stmt(inst);
  inst->ps.inside_loop = false;
  if (body == NULL) {
    une_node_free(counter, false);
    une_node_free(from, false);
    une_node_free(to, false);
    return NULL;
  }
  
  une_node *node = une_node_create(UNE_NT_FOR);
  node->pos = (une_position){
    .start = pos_start,
    .end = inst->ps.tokens[inst->ps.index-1].pos.end // FIXME: Token Stream Helper Function?
  };
  node->content.branch.a = counter;
  node->content.branch.b = from;
  node->content.branch.c = to;
  node->content.branch.d = body;
  return node;
}
#pragma endregion une_parse_for

#pragma region une_parse_while
static une_node *une_parse_while(une_instance *inst) {
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:while [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  size_t pos_start = une_p_peek(inst).pos.start;
  
  // [While]
  une_p_consume(inst);
  
  // [Condition]
  une_node *condition = une_parse_expression(inst);
  if (condition == NULL) return NULL;
  
  // [Body]
  inst->ps.inside_loop = true;
  une_node *body = une_parse_stmt(inst);
  inst->ps.inside_loop = false;
  if (body == NULL) {
    une_node_free(condition, false);
    return NULL;
  }
  
  une_node *node = une_node_create(UNE_NT_WHILE);
  node->pos = (une_position){
    .start = pos_start,
    .end = body->pos.end
  };
  node->content.branch.a = condition;
  node->content.branch.b = body;
  return node;
}
#pragma endregion une_parse_while

#pragma region une_parse_def
static une_node *une_parse_def(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:def [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  // [Def]
  size_t pos_start = une_p_peek(inst).pos.start;
  une_p_consume(inst);

  // [Id]
  une_node *name = une_parse_id(inst);
  if (name == NULL) return NULL;

  // [Params]
  une_node *params = une_parse_sequence(
    inst,
    UNE_NT_LIST,
    UNE_TT_LPAR, UNE_TT_SEP, UNE_TT_RPAR,
    &une_parse_id
  );
  if (params == NULL) {
    une_node_free(name, false);
    return NULL;
  }
  
  // [Body]
  une_node *body = une_parse_stmt(inst);
  if (body == NULL) {
    une_node_free(name, false);
    une_node_free(params, false);
    return NULL;
  }

  une_node *def = une_node_create(UNE_NT_DEF);
  def->pos = (une_position){
    .start = pos_start,
    .end = body->pos.end
  };
  def->content.branch.a = name;
  def->content.branch.b = params;
  def->content.branch.c = body;

  return def;
}
#pragma endregion une_parse_def

#pragma region une_parse_expression
static une_node *une_parse_expression(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:expression [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  // [Expression]
  une_node *exp_true = une_parse_and_or(inst);
  if (exp_true == NULL) return NULL;
  if (une_p_peek(inst).type != UNE_TT_IF) return exp_true;
  
// Conditional Operation

  // [If]
  une_p_consume(inst);
  
  // [Condition]
  une_node *cond = une_parse_and_or(inst);
  if (cond == NULL) {
    une_node_free(exp_true, false);
    return NULL;
  }
  
  // [Else]
  if (une_p_peek(inst).type != UNE_TT_ELSE) {
    inst->error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, une_p_peek(inst).pos,
                            _int=(int)UNE_TT_ELSE, _int=0);
    une_node_free(exp_true, false);
    une_node_free(cond, false);
    return NULL;
  }
  une_p_consume(inst);
  
  // [Expression]
  une_node *exp_false = une_parse_expression(inst);
  if (exp_false == NULL) {
    une_node_free(exp_true, false);
    une_node_free(cond, false);
    return NULL;
  }
  
  une_node *cop = une_node_create(UNE_NT_COP);
  cop->pos.start = exp_true->pos.start;
  cop->pos.end = exp_false->pos.end;
  cop->content.branch.a = exp_true;
  cop->content.branch.b = cond;
  cop->content.branch.c = exp_false;
  return cop;
}
#pragma endregion une_parse_add_sub

#pragma region une_parse_and_or
static une_node *une_parse_and_or(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:and_or [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  return une_parse_binary_operation(
    inst,
    UNE_TT_AND,
    UNE_NT_AND,
    UNE_TT_OR,
    UNE_NT_OR,
    &une_parse_condition,
    &une_parse_condition
  );
}
#pragma endregion une_parse_and_or

#pragma region une_parse_condition
static une_node *une_parse_condition(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:condition [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  if (une_p_peek(inst).type == UNE_TT_NOT) {
    #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
      LOG(L"parse:condition:not [%ls]", une_token_to_wcs(une_p_peek(inst)));
    #endif
    return une_parse_unary_operation(
      inst,
      UNE_NT_NOT,
      &une_parse_condition
    );
  }
  
  return une_parse_binary_operation(
    inst,
    UNE_TT_EQU,
    UNE_NT_EQU,
    UNE_TT_LEQ,
    UNE_NT_LEQ,
    &une_parse_add_sub,
    &une_parse_add_sub
  );
}
#pragma endregion une_parse_condition

#pragma region une_parse_add_sub
static une_node *une_parse_add_sub(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:add_sub [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  return une_parse_binary_operation(
    inst,
    UNE_TT_ADD,
    UNE_NT_ADD,
    UNE_TT_SUB,
    UNE_NT_SUB,
    &une_parse_term,
    &une_parse_term
  );
}
#pragma endregion une_parse_add_sub

#pragma region une_parse_term
static une_node *une_parse_term(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:term [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  return une_parse_binary_operation(
    inst,
    UNE_TT_MUL,
    UNE_NT_MUL,
    UNE_TT_MOD,
    UNE_NT_MOD,
    &une_parse_power,
    &une_parse_power
  );
}
#pragma endregion une_parse_term

#pragma region une_parse_power
static une_node *une_parse_power(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:power [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  return une_parse_binary_operation(
    inst,
    UNE_TT_POW,
    UNE_NT_POW,
    UNE_TT_POW,
    UNE_NT_POW,
    &une_parse_index,
    // DOC: We parse power and not index because powers are evaluated right to left.
    &une_parse_power
  );
}
#pragma endregion une_parse_power

#pragma region une_parse_index
static une_node *une_parse_index(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:index [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  une_node *left = une_parse_atom(inst);
  if (left == NULL) return NULL;
  
  while (une_p_peek(inst).type == UNE_TT_LSQB) {
    size_t pos_start = left->pos.start;
    une_p_consume(inst);

    une_node *right = une_parse_expression(inst);
    if (right == NULL) return NULL;
    if (une_p_peek(inst).type != UNE_TT_RSQB) {
      inst->error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, une_p_peek(inst).pos,
                              _int=(int)UNE_TT_RSQB, _int=0);
      return NULL;
    }
    
    une_node *new_left = une_node_create(UNE_NT_GET_IDX);
    new_left->pos = (une_position){
      .start = pos_start,
      .end = une_p_peek(inst).pos.end
    };
    une_p_consume(inst);
    new_left->content.branch.a = left;
    new_left->content.branch.b = right;
    left = new_left;
  }
  
  return left;
}
#pragma endregion une_parse_index

#pragma region une_parse_atom
static une_node *une_parse_atom(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:atom [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  switch (une_p_peek(inst).type) {
    
    #pragma region Negate
    case UNE_TT_SUB:
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
        LOG(L"parse:atom:negate [%ls]", une_token_to_wcs(une_p_peek(inst)));
      #endif
      return une_parse_unary_operation(
        inst,
        UNE_NT_NEG,
        &une_parse_atom
      );
    #pragma endregion Negate

    #pragma region Int
    case UNE_TT_INT: {
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
        LOG(L"parse:atom:int [%ls]", une_token_to_wcs(une_p_peek(inst)));
      #endif
      une_node *num = une_node_create(UNE_NT_INT);
      num->pos = une_p_peek(inst).pos;
      num->content.value._int = une_p_peek(inst).value._int;
      une_p_consume(inst);
      return num;
    }
    #pragma endregion Int
    
    #pragma region Flt
    case UNE_TT_FLT: {
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
        LOG(L"parse:atom:flt [%ls]", une_token_to_wcs(une_p_peek(inst)));
      #endif
      une_node *num = une_node_create(UNE_NT_FLT);
      num->pos = une_p_peek(inst).pos;
      num->content.value._flt = une_p_peek(inst).value._flt;
      une_p_consume(inst);
      return num;
    }
    #pragma endregion Flt
    
    #pragma region Str
    case UNE_TT_STR: {
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
        LOG(L"parse:atom:str [%ls]", une_token_to_wcs(une_p_peek(inst)));
      #endif
      une_node *str = une_node_create(UNE_NT_STR);
      str->pos = une_p_peek(inst).pos;
      /* DOC: Memory Management: This shows that nodes reference tokens' WCS
      instead of storing their own. */
      str->content.value._wcs = une_p_peek(inst).value._wcs;
      une_p_consume(inst);
      return str;
    }
    #pragma endregion Str
    
    #pragma region Get/Call
    case UNE_TT_ID: {
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
        LOG(L"parse:atom:get/call [%ls]", une_token_to_wcs(une_p_peek(inst)));
      #endif
      une_node *id = une_node_create(UNE_NT_ID);
      id->pos = une_p_peek(inst).pos;
      id->content.value._wcs = une_p_peek(inst).value._wcs;
      une_p_consume(inst);

      if (une_p_peek(inst).type != UNE_TT_LPAR) {
        #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
          LOG(L"parse:atom:get/call:get [%ls]", une_token_to_wcs(une_p_peek(inst)));
        #endif
        une_node *get = une_node_create(UNE_NT_GET);
        get->pos = id->pos;
        get->content.branch.a = id;
        return get;
      }

      // Function Call.
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
        LOG(L"parse:atom:get/call:call [%ls]", une_token_to_wcs(une_p_peek(inst)));
      #endif
      une_node *args = une_parse_sequence(
        inst,
        UNE_NT_LIST,
        UNE_TT_LPAR, UNE_TT_SEP, UNE_TT_RPAR,
        &une_parse_expression
      );
      if (args == NULL) {
        une_node_free(id, false);
        return NULL;
      }
      
      une_node *call = une_node_create(UNE_NT_CALL);
      call->pos = (une_position){
        .start = id->pos.start,
        .end = args->pos.end
      };
      call->content.branch.a = id;
      call->content.branch.b = args;
      return call;
    }
    #pragma endregion Get/Call

    #pragma region List
    case UNE_TT_LSQB:
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
        LOG(L"parse:atom:list [%ls]", une_token_to_wcs(une_p_peek(inst)));
      #endif
      return une_parse_sequence(
        inst,
        UNE_NT_LIST,
        UNE_TT_LSQB, UNE_TT_SEP, UNE_TT_RSQB,
        &une_parse_expression
      );
    #pragma endregion List
    
    #pragma region Expression
    case UNE_TT_LPAR: {
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
        LOG(L"parse:atom:expression [%ls]", une_token_to_wcs(une_p_peek(inst)));
      #endif
      une_p_consume(inst);
      une_node *expression = une_parse_expression(inst);
      if (expression == NULL) return NULL;
      if (une_p_peek(inst).type != UNE_TT_RPAR) {
        une_node_free(expression, false);
        inst->error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, une_p_peek(inst).pos,
                                _int=(int)UNE_TT_RPAR, _int=0);
        return NULL;
      }
      une_p_consume(inst);
      return expression;
    }
    #pragma endregion Expression
    
    default: {
      inst->error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_TOKEN, une_p_peek(inst).pos,
                              _int=(int)(une_p_peek(inst).type), _int=0);
      return NULL;
    }
  }
}
#pragma endregion une_parse_atom

#pragma region une_parse_id
static une_node *une_parse_id(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:id [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  if (une_p_peek(inst).type != UNE_TT_ID) {
    inst->error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, une_p_peek(inst).pos,
                            _int=UNE_TT_ID, _int=0);
    return NULL;
  }
  une_node *id = une_node_create(UNE_NT_ID);
  id->pos = une_p_peek(inst).pos;
  id->content.value._wcs = une_p_peek(inst).value._wcs;
  une_p_consume(inst);
  return id;
}
#pragma endregion une_parse_id

#pragma region une_parse_return
static une_node *une_parse_return(une_instance *inst)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:return [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  une_position pos = une_p_peek(inst).pos; /* If the parser finds a return
                                                  value after this, pos.end is
                                                  changed again further down. */
  une_p_consume(inst); // RETURN

  // Return Value.
  une_node *value = NULL; /* DOC: NULL here means no return value was specified.
                             This tells the interpreter that there is no return
                             value. */
  if (
    une_p_peek(inst).type != UNE_TT_NEW &&
    une_p_peek(inst).type != UNE_TT_EOF &&
    une_p_peek(inst).type != UNE_TT_RBRC
  ) {
    value = une_parse_expression(inst);
    if (value == NULL) return value; /* DOC: NULL here means an error. */
    pos.end = value->pos.end;
  }

  une_node *return_ = une_node_create(UNE_NT_RETURN);
  return_->pos = pos;
  return_->content.branch.a = value;
  return return_;
}
#pragma endregion une_parse_return

#pragma region une_parse_binary_operation
static une_node *une_parse_binary_operation(
  une_instance *inst,
  une_token_type range_begin_tt,
  une_node_type range_begin_nt,
  une_token_type range_end_tt,
  une_node_type range_end_nt,
  une_node* (*parse_left)(une_instance*),
  une_node* (*parse_right)(une_instance*)
)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:binary_operation [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  une_node *left = (*parse_left)(inst);
  if (left == NULL) return NULL;

  while (
    une_p_peek(inst).type >= range_begin_tt &&
    une_p_peek(inst).type <= range_end_tt
  )
  {
    une_node_type type = range_begin_nt+une_p_peek(inst).type-range_begin_tt;

    une_p_consume(inst);

    une_node *right = (*parse_right)(inst);
    if (right == NULL) {
      une_node_free(left, false);
      return NULL;
    }

    une_node *new_left = une_node_create(type);
    new_left->pos = (une_position){
      .start = left->pos.start,
      .end = right->pos.end
    };
    new_left->content.branch.a = left;
    new_left->content.branch.b = right;
    left = new_left;
  }
  
  return left;
}
#pragma endregion une_parse_binary_operation

#pragma region une_parse_unary_operation
static une_node *une_parse_unary_operation(
  une_instance *inst,
  une_node_type node_t,
  une_node* (*parse)(une_instance*)
)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
    LOG(L"parse:unary_operation [%ls]", une_token_to_wcs(une_p_peek(inst)));
  #endif
  
  size_t pos_start = une_p_peek(inst).pos.start;
  
  une_p_consume(inst);
  
  une_node *node = (*parse)(inst);
  if (node == NULL) return NULL;
  
  une_node *unop = une_node_create(node_t);
  unop->pos = (une_position){
    .start = pos_start,
    .end = node->pos.end
  };
  unop->content.branch.a = node;
  return unop;
}
#pragma endregion une_parse_unary_operation
