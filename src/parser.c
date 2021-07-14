/*
parser.c - Une
Modified 2021-07-14
*/

/* Header-specific includes. */
#include "parser.h"

/* Implementation-specific includes. */
#include "tools.h"
#include "stream.h"

/*
Public parser interface.
*/

UNE_ISTREAM_ARRAY_PULLER_VAL(pull, une_token, une_token, une_token_create(__UNE_TT_none__), false)
UNE_ISTREAM_ARRAY_PEEKER_VAL(peek, une_token, une_token, une_token_create(__UNE_TT_none__), false)
UNE_ISTREAM_ARRAY_ACCESS_VAL(now, une_token, une_token, une_token_create(__UNE_TT_none__), false)

une_node *une_parse(une_error *error, une_parser_state *ps, une_token *tokens)
{
  /* Initialize une_parser_state. */
  ps->in = une_istream_array_create((void*)tokens, 0);
  pull(&ps->in);

  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  return une_parse_sequence(error, ps, UNE_NT_STMTS, __UNE_TT_none__, UNE_TT_NEW, UNE_TT_EOF, &une_parse_stmt);
}

/*
Parse statement.
*/
__une_parser(une_parse_stmt)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  /* Skip NEW before/between statements. */
  if (now(&ps->in).type == UNE_TT_NEW)
    pull(&ps->in);
  assert(now(&ps->in).type != UNE_TT_NEW);
  
  switch (now(&ps->in).type) {
    case UNE_TT_LBRC:
      return une_parse_block(error, ps);
    case UNE_TT_DEF:
      return une_parse_def(error, ps);
    case UNE_TT_FOR:
      return une_parse_for(error, ps);
    case UNE_TT_WHILE:
      return une_parse_while(error, ps);
    case UNE_TT_IF:
      return une_parse_if(error, ps);
    case UNE_TT_CONTINUE:
      return une_parse_continue(error, ps);
    case UNE_TT_BREAK:
      return une_parse_break(error, ps);
    case UNE_TT_RETURN:
      return une_parse_return(error, ps);
    default:
      return une_parse_set_expstmt(error, ps);
  }

}

/*
Parse id.
*/
__une_parser(une_parse_id)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  if (now(&ps->in).type != UNE_TT_ID) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    return NULL;
  }

  une_node *id = une_node_create(UNE_NT_ID);
  id->pos = now(&ps->in).pos;
  id->content.value._wcs = now(&ps->in).value._wcs;
  pull(&ps->in);
  return id;
}

/*
Parse block.
*/
__une_parser(une_parse_block)
{
  return une_parse_sequence(error, ps, UNE_NT_STMTS, UNE_TT_LBRC, UNE_TT_NEW, UNE_TT_RBRC, &une_parse_stmt);
}

/*
Parse expression.
*/
__une_parser(une_parse_expression)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  /* Condition. */
  une_node *cond = une_parse_and_or(error, ps);
  if (cond == NULL || now(&ps->in).type != UNE_TT_QMARK)
    return cond;

  /* ?. */
  pull(&ps->in);
  
  /* Expression. */
  une_node *exp_true = une_parse_and_or(error, ps);
  if (exp_true == NULL) {
    une_node_free(cond, false);
    return NULL;
  }
  
  /* :. */
  if (now(&ps->in).type != UNE_TT_COLON) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    une_node_free(cond, false);
    une_node_free(exp_true, false);
    return NULL;
  }
  pull(&ps->in);
  
  /* Expression. */
  une_node *exp_false = une_parse_expression(error, ps);
  if (exp_false == NULL) {
    une_node_free(exp_true, false);
    une_node_free(cond, false);
    return NULL;
  }
  
  une_node *cop = une_node_create(UNE_NT_COP);
  cop->pos = (une_position){
    .start = cond->pos.start,
    .end = exp_false->pos.end
  };
  cop->content.branch.a = cond;
  cop->content.branch.b = exp_true;
  cop->content.branch.c = exp_false;
  return cop;
}

/*
Parse and/or.
*/
__une_parser(une_parse_and_or)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  return une_parse_binary_operation(error, ps,
    UNE_R_BGN_AND_OR_TOKENS,
    UNE_R_BGN_AND_OR_NODES,
    UNE_R_END_AND_OR_TOKENS,
    UNE_R_END_AND_OR_NODES,
    &une_parse_condition,
    &une_parse_condition
  );
}

/*
Parse condition.
*/
__une_parser(une_parse_condition)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  if (now(&ps->in).type == UNE_TT_NOT) {
    LOGPARSE(L"not", une_token_to_wcs(now(&ps->in)));
    return une_parse_unary_operation(error, ps, UNE_NT_NOT, &une_parse_condition);
  }
  
  return une_parse_binary_operation(error, ps,
    UNE_R_BGN_CONDITION_TOKENS,
    UNE_R_BGN_CONDITION_NODES,
    UNE_R_END_CONDITION_TOKENS,
    UNE_R_END_CONDITION_NODES,
    &une_parse_add_sub,
    &une_parse_add_sub
  );
}

/*
Parse add/sub.
*/
__une_parser(une_parse_add_sub)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  return une_parse_binary_operation(error, ps,
    UNE_R_BGN_ADD_SUB_TOKENS,
    UNE_R_BGN_ADD_SUB_NODES,
    UNE_R_END_ADD_SUB_TOKENS,
    UNE_R_END_ADD_SUB_NODES,
    &une_parse_term,
    &une_parse_term
  );
}

/*
Parse term.
*/
__une_parser(une_parse_term)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  return une_parse_binary_operation(error, ps,
    UNE_R_BGN_TERM_TOKENS,
    UNE_R_BEGIN_TERM_NODES,
    UNE_R_END_TERM_TOKENS,
    UNE_R_END_TERM_NODES,
    &une_parse_power,
    &une_parse_power
  );
}

/*
Parse power.
*/
__une_parser(une_parse_power)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  return une_parse_binary_operation(error, ps,
    UNE_R_BGN_POWER_TOKENS,
    UNE_R_BGN_POWER_NODES,
    UNE_R_END_POWER_TOKENS,
    UNE_R_END_POWER_NODES,
    &une_parse_index,
    &une_parse_power /* DOC: We parse power and not index because powers are evaluated right to left. */
  );
}

/*
Parse index.
*/
__une_parser(une_parse_index)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  /* Base. */
  une_node *left = une_parse_atom(error, ps);
  if (left == NULL)
    return NULL;
  
  /* Index. */
  while (now(&ps->in).type == UNE_TT_LSQB) {
    size_t pos_start = left->pos.start;
    pull(&ps->in);

    une_node *right = une_parse_expression(error, ps);
    if (right == NULL)
      return NULL;
    if (now(&ps->in).type != UNE_TT_RSQB) {
      *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
      return NULL;
    }
    
    une_node *new_left = une_node_create(UNE_NT_GET_IDX);
    new_left->pos = (une_position){
      .start = pos_start,
      .end = now(&ps->in).pos.end
    };
    pull(&ps->in);
    new_left->content.branch.a = left;
    new_left->content.branch.b = right;
    left = new_left;
  }
  
  return left;
}

/*
Parse atom.
*/
__une_parser(une_parse_atom)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  switch (now(&ps->in).type) {
    
    /* Negation. */
    case UNE_TT_SUB:
      LOGPARSE(L"negate", une_token_to_wcs(now(&ps->in)));
      return une_parse_unary_operation(error, ps, UNE_NT_NEG, &une_parse_atom);
    
    /* Numbers. */
    case UNE_TT_INT: {
      LOGPARSE(L"int", une_token_to_wcs(now(&ps->in)));
      une_node *num = une_node_create(UNE_NT_INT);
      num->pos = now(&ps->in).pos;
      num->content.value._int = now(&ps->in).value._int;
      pull(&ps->in);
      return num;
    }
    case UNE_TT_FLT: {
      LOGPARSE(L"flt", une_token_to_wcs(now(&ps->in)));
      une_node *num = une_node_create(UNE_NT_FLT);
      num->pos = now(&ps->in).pos;
      num->content.value._flt = now(&ps->in).value._flt;
      pull(&ps->in);
      return num;
    }
    
    /* String. */
    case UNE_TT_STR: {
      LOGPARSE(L"str", une_token_to_wcs(now(&ps->in)));
      une_node *str = une_node_create(UNE_NT_STR);
      str->pos = now(&ps->in).pos;
      /* DOC: Memory Management: This shows that nodes reference tokens' WCS
      instead of storing their own. */
      str->content.value._wcs = now(&ps->in).value._wcs;
      pull(&ps->in);
      return str;
    }
    
    /* Variable or function call. */
    case UNE_TT_ID: {
      LOGPARSE(L"get/call", une_token_to_wcs(now(&ps->in)));
      une_node *id = une_node_create(UNE_NT_ID);
      id->pos = now(&ps->in).pos;
      id->content.value._wcs = now(&ps->in).value._wcs;
      pull(&ps->in);

      if (now(&ps->in).type != UNE_TT_LPAR) {
        LOGPARSE(L"get/call:get", une_token_to_wcs(now(&ps->in)));
        une_node *get = une_node_create(UNE_NT_GET);
        get->pos = id->pos;
        get->content.branch.a = id;
        return get;
      }

      /* Function Call. */
      LOGPARSE(L"get/call:call", une_token_to_wcs(now(&ps->in)));
      une_node *args = une_parse_sequence(error, ps, UNE_NT_LIST, UNE_TT_LPAR, UNE_TT_SEP, UNE_TT_RPAR, &une_parse_expression);
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
    
    /* List. */
    case UNE_TT_LSQB:
      LOGPARSE(L"list", une_token_to_wcs(now(&ps->in)));
      return une_parse_sequence(error, ps, UNE_NT_LIST, UNE_TT_LSQB, UNE_TT_SEP, UNE_TT_RSQB, &une_parse_expression);
    
    /* Expression. */
    case UNE_TT_LPAR: {
      LOGPARSE(L"expression", une_token_to_wcs(now(&ps->in)));
      pull(&ps->in);
      une_node *expression = une_parse_expression(error, ps);
      if (expression == NULL)
        return NULL;
      if (now(&ps->in).type != UNE_TT_RPAR) {
        une_node_free(expression, false);
        *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
        return NULL;
      }
      pull(&ps->in);
      return expression;
    }
    
    default:
      *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
      return NULL;
  
  }
}

/*
Parse function definition.
*/
__une_parser(une_parse_def)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  /* Def. */
  size_t pos_start = now(&ps->in).pos.start;
  pull(&ps->in);

  /* Id. */
  une_node *name = une_parse_id(error, ps);
  if (name == NULL)
    return NULL;

  /* Params. */
  une_node *params = une_parse_sequence(error, ps, UNE_NT_LIST, UNE_TT_LPAR, UNE_TT_SEP, UNE_TT_RPAR, &une_parse_id);
  if (params == NULL) {
    une_node_free(name, false);
    return NULL;
  }
  
  /* Body. */
  une_node *body = une_parse_stmt(error, ps);
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

/*
Parse 'for' loop.
*/
__une_parser(une_parse_for)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  size_t pos_start = now(&ps->in).pos.start;
  
  /* For. */
  pull(&ps->in);
  
  /* Id. */
  une_node *counter = une_parse_id(error, ps);
  if (counter == NULL)
    return NULL;
  
  /* From. */
  if (now(&ps->in).type != UNE_TT_FROM) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    une_node_free(counter, false);
    return NULL;
  }
  pull(&ps->in);
  
  /* Expression. */
  une_node *from = une_parse_expression(error, ps);
  if (from == NULL) {
    une_node_free(counter, false);
    return NULL;
  }
  
  /* To. */
  if (now(&ps->in).type != UNE_TT_TILL) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    une_node_free(counter, false);
    une_node_free(from, false);
    return NULL;
  }
  pull(&ps->in);
  
  /* Expression. */
  une_node *to = une_parse_expression(error, ps);
  if (to == NULL) {
    une_node_free(counter, false);
    une_node_free(from, false);
    return NULL;
  }
  
  /* Body. */
  ps->loop_level++;
  une_node *body = une_parse_stmt(error, ps);
  ps->loop_level--;
  if (body == NULL) {
    une_node_free(counter, false);
    une_node_free(from, false);
    une_node_free(to, false);
    return NULL;
  }
  
  une_node *node = une_node_create(UNE_NT_FOR);
  node->pos = (une_position){
    .start = pos_start,
    .end = peek(&ps->in, -1).pos.end
  };
  node->content.branch.a = counter;
  node->content.branch.b = from;
  node->content.branch.c = to;
  node->content.branch.d = body;
  return node;
}

/*
Parse 'while' loop.
*/
__une_parser(une_parse_while)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  size_t pos_start = now(&ps->in).pos.start;
  
  /* While. */
  pull(&ps->in);
  
  /* Condition. */
  une_node *condition = une_parse_expression(error, ps);
  if (condition == NULL)
    return NULL;
  
  /* Body. */
  ps->loop_level++;
  une_node *body = une_parse_stmt(error, ps);
  ps->loop_level--;
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

/*
Parse 'if' statement.
*/
__une_parser(une_parse_if)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  size_t pos_start = now(&ps->in).pos.start;
  
  /* If || Elif. */
  pull(&ps->in);

  /* Condition. */
  une_node *predicate = une_parse_expression(error, ps);
  if (predicate == NULL)
    return NULL;
  
  /* Body. */
  une_node *consequent = une_parse_stmt(error, ps);
  if (consequent == NULL) {
    une_node_free(predicate, false);
    return NULL;
  }

  /* Whitespace. This counts for both 'elif' and 'else' because 'elif'
     creates an entirely new if node where this stmt then removes
     whitespace in front of 'else'.
     */
  size_t _token_index = ps->in.index; /* Here we skip over whitespace expecting
                                         an elif or else clause. If we don't
                                         find one, however, we have now skipped
                                         the whitespace that tells
                                         une_parse_sequence that a new command
                                         is starting. Therefore, we need to
                                         return back to this index in case there
                                         is no clause following the if clause.
                                         */
  if (now(&ps->in).type == UNE_TT_NEW)
    pull(&ps->in);

  une_node *ifstmt = une_node_create(UNE_NT_IF);
  ifstmt->pos.start = pos_start;
  ifstmt->content.branch.a = predicate;
  ifstmt->content.branch.b = consequent;

  /* Only If Body. */
  if (now(&ps->in).type != UNE_TT_ELSE && now(&ps->in).type != UNE_TT_ELIF) {
    ps->in.index = _token_index;
    ifstmt->pos.end = consequent->pos.end;
    return ifstmt;
  }
  
  /* Elif Body || Else Body. */
  une_node *alternate;
  if (now(&ps->in).type == UNE_TT_ELIF)
    alternate = une_parse_if(error, ps);
  else {
    pull(&ps->in);
    alternate = une_parse_stmt(error, ps);
  }
  if (alternate == NULL) {
    une_node_free(ifstmt, false);
    return NULL;
  }
  ifstmt->pos.end = alternate->pos.end;
  ifstmt->content.branch.c = alternate;
  return ifstmt;
}

/*
Parse continue.
*/
__une_parser(une_parse_continue)
{
  if (ps->loop_level == 0) {
    *error = UNE_ERROR_SET(UNE_ET_CONTINUE_OUTSIDE_LOOP, now(&ps->in).pos);
    return NULL;
  }
  une_node *continue_ = une_node_create(UNE_NT_CONTINUE);
  continue_->pos = now(&ps->in).pos;
  pull(&ps->in);
  return continue_;
}

/*
Parse break.
*/
__une_parser(une_parse_break)
{
  if (ps->loop_level == 0) {
    *error = UNE_ERROR_SET(UNE_ET_BREAK_OUTSIDE_LOOP, now(&ps->in).pos);
    return NULL;
  }
  une_node *break_ = une_node_create(UNE_NT_BREAK);
  break_->pos = now(&ps->in).pos;
  pull(&ps->in);
  return break_;
}

/*
Parse 'return' statement.
*/
__une_parser(une_parse_return)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  une_position pos = now(&ps->in).pos; /* If the parser finds a return
                                            value after this, pos.end is
                                            changed again further down. */
  pull(&ps->in); /* Return. */

  /* Return Value. */
  une_node *value = NULL; /* DOC: NULL here means no return value was specified.
                             This tells the interpreter that there is no return
                             value. */
  if (now(&ps->in).type != UNE_TT_NEW && now(&ps->in).type != UNE_TT_EOF && now(&ps->in).type != UNE_TT_RBRC) {
    value = une_parse_expression(error, ps);
    if (value == NULL) /* DOC: NULL here means an error. */
      return value;
    pos.end = value->pos.end;
  }

  une_node *return_ = une_node_create(UNE_NT_RETURN);
  return_->pos = pos;
  return_->content.branch.a = value;
  return return_;
}

/*
Parse set/expression statement.
*/
__une_parser(une_parse_set_expstmt)
{
  /* Check for global variable definition. */
  bool global = false;
  if (now(&ps->in).type == UNE_TT_GLOBAL) {
    pull(&ps->in);
    global = true;
  }
  
  /* Check for variable definition. */
  if (now(&ps->in).type == UNE_TT_ID) {
    size_t token_index_before = ps->in.index; /* Needed to backstep in case this is not a variable definition. */

    une_node *target = une_parse_id(error, ps);
    if (target == NULL)
      return NULL;

    une_node *varset = une_node_create(UNE_NT_SET);
    varset->pos.start = target->pos.start;
    varset->content.branch.d = (une_node*)global;
    varset->content.branch.a = target;

    /* Index. */
    if (now(&ps->in).type == UNE_TT_LSQB) {
      varset->type = UNE_NT_SET_IDX;
      pull(&ps->in);

      une_node *position = une_parse_expression(error, ps);
      if (position == NULL) {
        une_node_free(varset, false);
        return NULL;
      }
      varset->content.branch.b = position;

      if (now(&ps->in).type != UNE_TT_RSQB) {
        une_node_free(varset, false);
        return NULL;
      }
      pull(&ps->in);
    }

    /* Expression. */
    if (now(&ps->in).type == UNE_TT_SET) {
      pull(&ps->in);

      une_node *expression = une_parse_expression(error, ps);
      if (expression == NULL) {
        une_node_free(varset, false);
        return NULL;
      }

      varset->pos.end = expression->pos.end;
      if (varset->type == UNE_NT_SET_IDX)
        varset->content.branch.c = expression;
      else
        varset->content.branch.b = expression;
      return varset;
    }

    /* Not a Variable Definition -> Expression. */
    une_node_free(varset, false);
    ps->in.index = token_index_before;
  }

  return une_parse_expression(error, ps);
}

/*
Parse unary operation.
*/
__une_parser(une_parse_unary_operation, une_node_type node_t, une_node* (*parse)(une_error*, une_parser_state*)
)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  size_t pos_start = now(&ps->in).pos.start;
  
  pull(&ps->in);
  
  une_node *node = (*parse)(error, ps);
  if (node == NULL)
    return NULL;
  
  une_node *unop = une_node_create(node_t);
  unop->pos = (une_position){
    .start = pos_start,
    .end = node->pos.end
  };
  unop->content.branch.a = node;
  return unop;
}

/*
Parse binary operation.
*/
__une_parser(une_parse_binary_operation,
  une_token_type range_begin_tt,
  une_node_type range_begin_nt,
  une_token_type range_end_tt,
  une_node_type range_end_nt,
  une_node* (*parse_left)(une_error*, une_parser_state*),
  une_node* (*parse_right)(une_error*, une_parser_state*)
)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  une_node *left = (*parse_left)(error, ps);
  if (left == NULL)
    return NULL;

  while (now(&ps->in).type >= range_begin_tt && now(&ps->in).type <= range_end_tt)
  {
    une_node_type type = range_begin_nt+now(&ps->in).type-range_begin_tt;

    pull(&ps->in);

    une_node *right = (*parse_right)(error, ps);
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

/*
Parse sequence.
*/
__une_parser(une_parse_sequence,
  une_node_type node_type,
  une_token_type tt_begin, une_token_type tt_end_of_item, une_token_type tt_end,
  une_node* (*parser)(une_error*, une_parser_state*)
)
{
  LOGPARSE(L"", une_token_to_wcs(now(&ps->in)));
  
  /* Begin Sequence. */
  size_t pos_start = now(&ps->in).pos.start;
  if (tt_begin != __UNE_TT_none__) {
    if (now(&ps->in).type != tt_begin) {
      *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
      return NULL;
    }
    pull(&ps->in);
  }
  
  /* Sequence. */
  size_t sequence_size = UNE_SIZE_SEQUENCE;
  une_node **sequence = malloc(sequence_size*sizeof(*sequence));
  size_t sequence_index = 1;

  while (true) {
    if (sequence_index >= sequence_size) {
      sequence_size *= 2;
      sequence = realloc(sequence, sequence_size*sizeof(sequence));
    }
    
    /* ADDITIONAL WHITESPACE. */
    while (now(&ps->in).type == tt_end_of_item || now(&ps->in).type == UNE_TT_NEW)
      pull(&ps->in);

    /* EXPECTED END OF SEQUENCE. */
    if (now(&ps->in).type == tt_end)
      break;

    /* UNEXPECTED END OF SEQUENCE. */
    if (now(&ps->in).type == UNE_TT_EOF) {
      /* This can happen if a block, list, list of parameters, or list of
      arguments is opened at the end of the file without being closed. */
      for (size_t i=1; i<sequence_index; i++)
        une_node_free(sequence[i], false);
      free(sequence);
      *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
      return NULL;
    }

    /* PARSE ITEM. */
    sequence[sequence_index] = (*parser)(error, ps);
    if (sequence[sequence_index] == NULL) {
      for (size_t i=1; i<sequence_index; i++)
        une_node_free(sequence[i], false);
      free(sequence);
      return NULL;
    }
    sequence_index++;
    
    /* ITEM DELIMITER. */
    if (now(&ps->in).type == tt_end || now(&ps->in).type == tt_end_of_item || now(&ps->in).type == UNE_TT_NEW)
      continue;
    for (size_t i=1; i<sequence_index; i++)
      une_node_free(sequence[i], false);
    free(sequence);
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    return NULL;
  }

  /* CREATE NODE. */
  une_node *counter = une_node_create(UNE_NT_SIZE);
  counter->content.value._int = sequence_index-1;
  sequence[0] = counter;
  une_node *node = une_node_create(node_type);
  node->pos = (une_position){
    .start = pos_start,
    .end = now(&ps->in).pos.end
  };
  node->content.value._vpp = (void**)sequence;
  
  /* End Sequence. */
  /* We don't skip EOF because it may still be needed by other functions up
  the call chain. */
  if (tt_end != UNE_TT_EOF)
    pull(&ps->in);
  return node;
}
