/*
parser.c - Une
Updated 2021-05-03
*/

#include "parser.h"

#pragma region une_parse
une_node *une_parse(une_token *tokens, size_t *token_index, une_error *error)
{
  return une_parse_sequence(
    tokens, token_index, error,
    UNE_NT_STMTS,
    __UNE_TT_none__, UNE_TT_NEW, UNE_TT_EOF,
    &une_parse_stmt
  );
}
#pragma endregion une_parse

#pragma region une_parse_sequence
static une_node *une_parse_sequence(
  une_token *tokens, size_t *token_index, une_error *error,
  une_node_type node_type,
  une_token_type tt_begin, une_token_type tt_end_of_item, une_token_type tt_end,
  une_node* (*parser)(une_token*, size_t*, une_error*)
)
{
  // [Begin Sequence]
  if (tt_begin != __UNE_TT_none__) {
    if (tokens[*token_index].type != tt_begin) {
      *error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, tokens[*token_index].pos,
                              _int=(int)tt_begin, _int=0);
      return NULL;
    }
    (*token_index)++;
  }
  
  // [Sequence]
  size_t pos_start = tokens[*token_index].pos.start;
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
      tokens[*token_index].type == tt_end_of_item ||
      tokens[*token_index].type == UNE_TT_NEW
    ) (*token_index)++;

    // EXPECTED END OF SEQUENCE
    if (tokens[*token_index].type == tt_end) break;

    // UNEXPECTED END OF SEQUENCE
    if (tokens[*token_index].type == UNE_TT_EOF) {
      /* This can happen if a block, list, list of parameters, or list of
      arguments is opened at the end of the file without being closed. */
      for (size_t i=0; i<sequence_index; i++) une_node_free(sequence[i], false);
      free(sequence);
      *error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_TOKEN, tokens[*token_index].pos,
                              _int=(int)UNE_TT_EOF, _int=0);
      return NULL;
    }

    // PARSE ITEM
    sequence[sequence_index] = (*parser)(tokens, token_index, error);
    if (sequence[sequence_index] == NULL) {
      for (size_t i=0; i<sequence_index; i++) une_node_free(sequence[i], false);
      free(sequence);
      return NULL;
    }
    sequence_index++;
    
    // ITEM DELIMITER
    if (
      tokens[*token_index].type == tt_end ||
      tokens[*token_index].type == tt_end_of_item ||
      tokens[*token_index].type == UNE_TT_NEW
    ) continue;
    for (size_t i=0; i<sequence_index; i++) une_node_free(sequence[i], false);
    free(sequence);
    *error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, tokens[*token_index].pos,
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
    .end = tokens[*token_index].pos.end
  };
  node->content.value._vpp = (void**)sequence;
  
  // [End Sequence]
  /* We don't skip EOF because it may still be needed by other functions up
  the call chain. */
  if (tt_end != UNE_TT_EOF) (*token_index)++;
  
  return node;
}
#pragma endregion une_parse_sequence

#pragma region une_parse_stmt
static une_node *une_parse_stmt(
  une_token *tokens,
  size_t *token_index,
  une_error *error
  /*bool is_loop_body*/
)
{
  if (tokens[*token_index].type == UNE_TT_NEW) (*token_index)++;
  
  switch (tokens[*token_index].type) {
    case UNE_TT_FOR: return une_parse_for(tokens, token_index, error);
    case UNE_TT_IF: return une_parse_if(tokens, token_index, error);
    case UNE_TT_WHILE: return une_parse_while(tokens, token_index, error);
    case UNE_TT_DEF: return une_parse_def(tokens, token_index, error);
    case UNE_TT_RETURN: return une_parse_return(tokens, token_index, error);
    
    #pragma region Block
    case UNE_TT_LBRC: return une_parse_sequence(
      tokens, token_index, error,
      UNE_NT_STMTS,
      UNE_TT_LBRC, UNE_TT_NEW, UNE_TT_RBRC,
      &une_parse_stmt
    );
    #pragma endregion Block
    
    #pragma region Break
    case UNE_TT_BREAK: {
      // if (!is_loop_body) {
      //   *error = UNE_ERROR_SETX(UNE_ET_BREAK_OUTSIDE_LOOP, tokens[*token_index].pos);
      //   return NULL;
      // }
      une_node *break_ = une_node_create(UNE_NT_BREAK);
      break_->pos = tokens[*token_index].pos;
      (*token_index)++;
      return break_;
    }
    #pragma endregion Break
    
    #pragma region Continue
    case UNE_TT_CONTINUE: {
      // if (!is_loop_body) {
      //   *error = UNE_ERROR_SETX(UNE_ET_CONTINUE_OUTSIDE_LOOP, tokens[*token_index].pos);
      //   return NULL;
      // }
      une_node *continue_ = une_node_create(UNE_NT_CONTINUE);
      continue_->pos = tokens[*token_index].pos;
      (*token_index)++;
      return continue_;
    }
    #pragma endregion Continue
    
    #pragma region Variable Definition
    case UNE_TT_ID: {
      // [Id]
      size_t token_index_before = *token_index; // Needed to backstep in case this is not a variable definition.
      
      une_node *target = une_parse_id(tokens, token_index, error);
      if (target == NULL) return NULL;
      
      une_node *varset = une_node_create(UNE_NT_SET);
      varset->pos.start = target->pos.start;
      varset->content.branch.a = target;
      
      // [Index]
      if (tokens[*token_index].type == UNE_TT_LSQB) {
        varset->type = UNE_NT_SET_IDX;
        (*token_index)++;
        
        une_node *position = une_parse_expression(tokens, token_index, error);
        if (position == NULL) {
          une_node_free(varset, false);
          return NULL;
        }
        varset->content.branch.b = position;
        
        if (tokens[*token_index].type != UNE_TT_RSQB) {
          une_node_free(varset, false);
          return NULL;
        }
        (*token_index)++;
      }
      
      // [Expression]
      if (tokens[*token_index].type == UNE_TT_SET) {
        (*token_index)++;
        
        une_node *expression = une_parse_expression(tokens, token_index, error);
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
      *token_index = token_index_before;
      /*
      Notice how there is no break here: If the above code ended here it means
      the stmt is not a variable definition, leaving the only option
      for it to be a conditional operation.
      */
    }
    #pragma endregion Variable Definition

    #pragma region Expression
    default: return une_parse_expression(tokens, token_index, error);
    #pragma endregion Expression
  }
}
#pragma endregion une_parse_stmt

#pragma region une_parse_if
static une_node *une_parse_if(une_token *tokens, size_t *token_index, une_error *error)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // [If || Elif]
  (*token_index)++;

  // [Condition]
  une_node *condition = une_parse_expression(tokens, token_index, error);
  if (condition == NULL) return NULL;
  
  // [Body]
  une_node *truebody = une_parse_stmt(tokens, token_index, error);
  if (truebody == NULL) {
    une_node_free(condition, false);
    return NULL;
  }

  /*
  Whitespace. This counts for both 'elif' and 'else' because 'elif'
  creates an entirely new if node where this stmt then removes
  whitespace in front of 'else'.
  */
  size_t _token_index = *token_index; /* Here we skip over whitespace expecting
                                         an elif or else clause. If we don't
                                         find one, however, we have now skipped
                                         the whitespace that tells
                                         une_parse_sequence that a new command
                                         is starting. Therefore, we need to
                                         return back to this index in case there
                                         is no clause following the if clause.
                                         */
  if (tokens[*token_index].type == UNE_TT_NEW) (*token_index)++;

  une_node *ifstmt = une_node_create(UNE_NT_IF);
  ifstmt->pos.start = pos_start;
  ifstmt->content.branch.a = condition;
  ifstmt->content.branch.b = truebody;

  // Only If Body
  if (
    tokens[*token_index].type != UNE_TT_ELSE &&
    tokens[*token_index].type != UNE_TT_ELIF
  ) {
    *token_index = _token_index;
    ifstmt->pos.end = truebody->pos.end;
    return ifstmt;
  }

  // [Elif Body || Else Body]
  une_node *falsebody = une_parse_if(tokens, token_index, error);
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
static une_node *une_parse_for(une_token *tokens, size_t *token_index, une_error *error) {
  size_t pos_start = tokens[*token_index].pos.start;
  
  // [For]
  (*token_index)++;
  
  // [Id]
  une_node *counter = une_parse_id(tokens, token_index, error);
  if (counter == NULL) return NULL;
  
  // [From]
  if (tokens[*token_index].type != UNE_TT_FROM) {
    *error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, tokens[*token_index].pos,
                            _int=UNE_TT_FROM, _int=0);
    une_node_free(counter, false);
    return NULL;
  }
  (*token_index)++;
  
  // [Expression]
  une_node *from = une_parse_expression(tokens, token_index, error);
  if (from == NULL) {
    une_node_free(counter, false);
    return NULL;
  }
  
  // [To]
  if (tokens[*token_index].type != UNE_TT_TILL) {
    *error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, tokens[*token_index].pos,
                            _int=UNE_TT_TILL, _int=0);
    une_node_free(counter, false);
    une_node_free(from, false);
    return NULL;
  }
  (*token_index)++;
  
  // [Expression]
  une_node *to = une_parse_expression(tokens, token_index, error);
  if (to == NULL) {
    une_node_free(counter, false);
    une_node_free(from, false);
    return NULL;
  }
  
  // [Body]
  une_node *body = une_parse_stmt(tokens, token_index, error);
  if (body == NULL) {
    une_node_free(counter, false);
    une_node_free(from, false);
    une_node_free(to, false);
    return NULL;
  }
  
  une_node *node = une_node_create(UNE_NT_FOR);
  node->pos = (une_position){
    .start = pos_start,
    .end = tokens[(*token_index)-1].pos.end
  };
  node->content.branch.a = counter;
  node->content.branch.b = from;
  node->content.branch.c = to;
  node->content.branch.d = body;
  return node;
}
#pragma endregion une_parse_for

#pragma region une_parse_while
static une_node *une_parse_while(une_token *tokens, size_t *token_index, une_error *error) {
  size_t pos_start = tokens[*token_index].pos.start;
  
  // [While]
  (*token_index)++;
  
  // [Condition]
  une_node *condition = une_parse_expression(tokens, token_index, error);
  if (condition == NULL) return NULL;
  
  // [Body]
  une_node *body = une_parse_stmt(tokens, token_index, error);
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
static une_node *une_parse_def(une_token *tokens, size_t *token_index, une_error *error)
{
  // [Def]
  size_t pos_start = tokens[*token_index].pos.start;
  (*token_index)++;

  // [Id]
  une_node *name = une_parse_id(tokens, token_index, error);
  if (name == NULL) return NULL;

  // [Params]
  une_node *params = une_parse_sequence(
    tokens, token_index, error,
    UNE_NT_LIST,
    UNE_TT_LPAR, UNE_TT_SEP, UNE_TT_RPAR,
    &une_parse_id
  );
  if (params == NULL) {
    une_node_free(name, false);
    return NULL;
  }
  
  // [Body]
  une_node *body = une_parse_stmt(tokens, token_index, error);
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
static une_node *une_parse_expression(une_token *tokens, size_t *token_index, une_error *error)
{
  // [Expression]
  une_node *exp_true = une_parse_and_or(tokens, token_index, error);
  if (exp_true == NULL) return NULL;
  if (tokens[*token_index].type != UNE_TT_IF) return exp_true;
  
// Conditional Operation

  // [If]
  (*token_index)++;
  
  // [Condition]
  une_node *cond = une_parse_and_or(tokens, token_index, error);
  if (cond == NULL) {
    une_node_free(exp_true, false);
    return NULL;
  }
  
  // [Else]
  if (tokens[*token_index].type != UNE_TT_ELSE) {
    *error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, tokens[*token_index].pos,
                            _int=(int)UNE_TT_ELSE, _int=0);
    une_node_free(exp_true, false);
    une_node_free(cond, false);
    return NULL;
  }
  (*token_index)++;
  
  // [Expression]
  une_node *exp_false = une_parse_expression(tokens, token_index, error);
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
static une_node *une_parse_and_or(une_token *tokens, size_t *token_index, une_error *error)
{
  return une_parse_binary_operation(
    tokens,
    token_index,
    error,
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
static une_node *une_parse_condition(une_token *tokens, size_t *token_index, une_error *error)
{
  if (tokens[*token_index].type == UNE_TT_NOT) {
    return une_parse_unary_operation(
      tokens, token_index, error,
      UNE_NT_NOT,
      &une_parse_condition
    );
  }
  
  return une_parse_binary_operation(
    tokens,
    token_index,
    error,
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
static une_node *une_parse_add_sub(une_token *tokens, size_t *token_index, une_error *error)
{
  return une_parse_binary_operation(
    tokens,
    token_index,
    error,
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
static une_node *une_parse_term(une_token *tokens, size_t *token_index, une_error *error)
{
  return une_parse_binary_operation(
    tokens,
    token_index,
    error,
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
static une_node *une_parse_power(une_token *tokens, size_t *token_index, une_error *error)
{
  return une_parse_binary_operation(
    tokens,
    token_index,
    error,
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
static une_node *une_parse_index(une_token *tokens, size_t *token_index, une_error *error)
{
  une_node *left = une_parse_atom(tokens, token_index, error);
  if (left == NULL) return NULL;
  
  while (tokens[*token_index].type == UNE_TT_LSQB) {
    size_t pos_start = left->pos.start;
    (*token_index)++;

    une_node *right = une_parse_expression(tokens, token_index, error);
    if (right == NULL) return NULL;
    if (tokens[*token_index].type != UNE_TT_RSQB) {
      *error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, tokens[*token_index].pos,
                              _int=(int)UNE_TT_RSQB, _int=0);
      return NULL;
    }
    
    une_node *new_left = une_node_create(UNE_NT_GET_IDX);
    new_left->pos = (une_position){
      .start = pos_start,
      .end = tokens[*token_index].pos.end
    };
    (*token_index)++;
    new_left->content.branch.a = left;
    new_left->content.branch.b = right;
    left = new_left;
  }
  
  return left;
}
#pragma endregion une_parse_index

#pragma region une_parse_atom
static une_node *une_parse_atom(une_token *tokens, size_t *token_index, une_error *error)
{
  switch (tokens[*token_index].type) {
    
    #pragma region Negate
    case UNE_TT_SUB: return une_parse_unary_operation(
      tokens, token_index, error,
      UNE_NT_NEG,
      &une_parse_atom
    );
    #pragma endregion Negate

    #pragma region Int
    case UNE_TT_INT: {
      une_node *num = une_node_create(UNE_NT_INT);
      num->pos = tokens[*token_index].pos;
      num->content.value._int = tokens[*token_index].value._int;
      (*token_index)++;
      return num;
    }
    #pragma endregion Int
    
    #pragma region Flt
    case UNE_TT_FLT: {
      une_node *num = une_node_create(UNE_NT_FLT);
      num->pos = tokens[*token_index].pos;
      num->content.value._flt = tokens[*token_index].value._flt;
      (*token_index)++;
      return num;
    }
    #pragma endregion Flt
    
    #pragma region Str
    case UNE_TT_STR: {
      une_node *str = une_node_create(UNE_NT_STR);
      str->pos = tokens[*token_index].pos;
      /* DOC: Memory Management: This shows that nodes reference tokens' WCS
      instead of storing their own. */
      str->content.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;
      return str;
    }
    #pragma endregion Str
    
    #pragma region Get/Call
    case UNE_TT_ID: {
      une_node *id = une_node_create(UNE_NT_ID);
      id->pos = tokens[*token_index].pos;
      id->content.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;

      if (tokens[*token_index].type != UNE_TT_LPAR) {
        une_node *get = une_node_create(UNE_NT_GET);
        get->pos = id->pos;
        get->content.branch.a = id;
        return get;
      }

      // Function Call.
      une_node *args = une_parse_sequence(
        tokens, token_index, error,
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
    case UNE_TT_LSQB: return une_parse_sequence(
        tokens, token_index, error,
        UNE_NT_LIST,
        UNE_TT_LSQB, UNE_TT_SEP, UNE_TT_RSQB,
        &une_parse_expression
      );
    #pragma endregion List
    
    #pragma region Expression
    case UNE_TT_LPAR: {
      (*token_index)++;
      une_node *expression = une_parse_expression(tokens, token_index, error);
      if (expression == NULL) return NULL;
      if (tokens[*token_index].type != UNE_TT_RPAR) {
        une_node_free(expression, false);
        *error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, tokens[*token_index].pos,
                                _int=(int)UNE_TT_RPAR, _int=0);
        return NULL;
      }
      (*token_index)++;
      return expression;
    }
    #pragma endregion Expression
    
    default: {
      *error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_TOKEN, tokens[*token_index].pos,
                              _int=(int)(tokens[*token_index].type), _int=0);
      return NULL;
    }
  }
}
#pragma endregion une_parse_atom

#pragma region une_parse_id
static une_node *une_parse_id(une_token *tokens, size_t *token_index, une_error *error)
{
  if (tokens[*token_index].type != UNE_TT_ID) {
    *error = UNE_ERROR_SETX(UNE_ET_EXPECTED_TOKEN, tokens[*token_index].pos,
                            _int=UNE_TT_ID, _int=0);
    return NULL;
  }
  une_node *id = une_node_create(UNE_NT_ID);
  id->pos = tokens[*token_index].pos;
  id->content.value._wcs = tokens[*token_index].value._wcs;
  (*token_index)++;
  return id;
}
#pragma endregion une_parse_id

#pragma region une_parse_return
static une_node *une_parse_return(une_token *tokens, size_t *token_index, une_error *error)
{
  une_position pos = tokens[*token_index].pos; /* If the parser finds a return
                                                  value after this, pos.end is
                                                  changed again further down. */
  (*token_index)++; // RETURN

  // Return Value.
  une_node *value = NULL; /* DOC: NULL here means no return value was specified.
                             This tells the interpreter that there is no return
                             value. */
  if (
    tokens[*token_index].type != UNE_TT_NEW &&
    tokens[*token_index].type != UNE_TT_EOF &&
    tokens[*token_index].type != UNE_TT_RBRC
  ) {
    value = une_parse_expression(tokens, token_index, error);
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
  une_token *tokens,
  size_t *token_index,
  une_error *error,
  une_token_type range_begin_tt,
  une_node_type range_begin_nt,
  une_token_type range_end_tt,
  une_node_type range_end_nt,
  une_node* (*parse_left)(une_token*, size_t*, une_error*),
  une_node* (*parse_right)(une_token*, size_t*, une_error*)
)
{
  une_node *left = (*parse_left)(tokens, token_index, error);
  if (left == NULL) return NULL;

  while (
    tokens[*token_index].type >= range_begin_tt &&
    tokens[*token_index].type <= range_end_tt
  )
  {
    une_node_type type = range_begin_nt+tokens[*token_index].type-range_begin_tt;

    (*token_index)++;

    une_node *right = (*parse_right)(tokens, token_index, error);
    if (right == NULL) return NULL;

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
  une_token *tokens,
  size_t *token_index,
  une_error *error,
  une_node_type node_t,
  une_node* (*parse)(une_token*, size_t*, une_error*)
)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  (*token_index)++;
  
  une_node *node = (*parse)(tokens, token_index, error);
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
