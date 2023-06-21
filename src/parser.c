/*
parser.c - Une
Modified 2023-06-21
*/

/* Header-specific includes. */
#include "parser.h"

/* Implementation-specific includes. */
#include "tools.h"
#include "stream.h"
#include "builtin_functions.h"

/*
Public parser interface.
*/

UNE_ISTREAM_ARRAY_PULLER_VAL(pull, une_token, une_token, une_token_create(UNE_TT_none__), false)
UNE_ISTREAM_ARRAY_PEEKER_VAL(peek, une_token, une_token, une_token_create(UNE_TT_none__), false)
UNE_ISTREAM_ARRAY_ACCESS_VAL(now, une_token, une_token, une_token_create(UNE_TT_none__), false)

une_node *une_parse(une_error *error, une_parser_state *ps, une_token *tokens)
{
  /* Initialize une_parser_state. */
  ps->in = une_istream_array_create((void*)tokens, 0);
  pull(&ps->in);

  LOGPARSE(L"", now(&ps->in));
  
  return une_parse_sequence(error, ps, UNE_NT_STMTS, UNE_TT_none__, UNE_TT_NEW, UNE_TT_EOF, &une_parse_stmt);
}

une_parser__(une_parse_stmt)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* Skip NEW before/between statements. */
  if (now(&ps->in).type == UNE_TT_NEW)
    pull(&ps->in);
  assert(now(&ps->in).type != UNE_TT_NEW);
  
  switch (now(&ps->in).type) {
    case UNE_TT_LBRC:
      return une_parse_block(error, ps);
    case UNE_TT_FOR:
      return une_parse_for(error, ps);
    case UNE_TT_WHILE:
      return une_parse_while(error, ps);
    case UNE_TT_IF:
      return une_parse_if(error, ps);
    case UNE_TT_ASSERT:
      return une_parse_assert(error, ps);
    case UNE_TT_CONTINUE:
      return une_parse_continue(error, ps);
    case UNE_TT_BREAK:
      return une_parse_break(error, ps);
    case UNE_TT_RETURN:
      return une_parse_return(error, ps);
    case UNE_TT_EXIT:
      return une_parse_exit(error, ps);
    default:
      break;
  }
  
  return une_parse_assignment_or_expr_stmt(error, ps);
}

une_parser__(une_parse_id)
{
  LOGPARSE(L"", now(&ps->in));
  
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

une_parser__(une_parse_block)
{
  return une_parse_sequence(error, ps, UNE_NT_STMTS, UNE_TT_LBRC, UNE_TT_NEW, UNE_TT_RBRC, &une_parse_stmt);
}

une_parser__(une_parse_expression)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* Condition. */
  une_node *cond = une_parse_and_or(error, ps);
  if (!cond || now(&ps->in).type != UNE_TT_QMARK)
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
  cop->pos = une_position_between(cond->pos, exp_false->pos);
  cop->content.branch.a = cond;
  cop->content.branch.b = exp_true;
  cop->content.branch.c = exp_false;
  return cop;
}

une_parser__(une_parse_and_or)
{
  LOGPARSE(L"", now(&ps->in));
  
  return une_parse_binary_operation(error, ps,
    UNE_R_BGN_AND_OR_TOKENS,
    UNE_R_BGN_AND_OR_NODES,
    UNE_R_END_AND_OR_TOKENS,
    &une_parse_condition,
    &une_parse_condition
  );
}

une_parser__(une_parse_condition)
{
  LOGPARSE(L"", now(&ps->in));
  
  if (now(&ps->in).type == UNE_TT_NOT) {
    LOGPARSE(L"not", now(&ps->in));
    return une_parse_unary_operation(error, ps, UNE_NT_NOT, &une_parse_condition);
  }
  
  return une_parse_binary_operation(error, ps,
    UNE_R_BGN_CONDITION_TOKENS,
    UNE_R_BGN_CONDITION_NODES,
    UNE_R_END_CONDITION_TOKENS,
    &une_parse_any_all,
    &une_parse_any_all
  );
}

une_parser__(une_parse_any_all)
{
  LOGPARSE(L"", now(&ps->in));
  
  if (now(&ps->in).type == UNE_TT_ANY)
    return une_parse_unary_operation(error, ps, UNE_NT_ANY, &une_parse_cover);
  if (now(&ps->in).type == UNE_TT_ALL)
    return une_parse_unary_operation(error, ps, UNE_NT_ALL, &une_parse_cover);
  return une_parse_cover(error, ps);
}

une_parser__(une_parse_cover)
{
  LOGPARSE(L"", now(&ps->in));
  
  return une_parse_binary_operation(error, ps,
    UNE_TT_COVER,
    UNE_NT_COVER,
    UNE_TT_COVER,
    &une_parse_add_sub,
    &une_parse_add_sub
  );
}

une_parser__(une_parse_add_sub)
{
  LOGPARSE(L"", now(&ps->in));
  
  return une_parse_binary_operation(error, ps,
    UNE_R_BGN_ADD_SUB_TOKENS,
    UNE_R_BGN_ADD_SUB_NODES,
    UNE_R_END_ADD_SUB_TOKENS,
    &une_parse_term,
    &une_parse_term
  );
}

une_parser__(une_parse_term)
{
  LOGPARSE(L"", now(&ps->in));
  
  return une_parse_binary_operation(error, ps,
    UNE_R_BGN_TERM_TOKENS,
    UNE_R_BEGIN_TERM_NODES,
    UNE_R_END_TERM_TOKENS,
    &une_parse_negation,
    &une_parse_negation
  );
}

une_parser__(une_parse_negation)
{
  LOGPARSE(L"", now(&ps->in));
  
  if (now(&ps->in).type == UNE_TT_SUB)
    return une_parse_unary_operation(error, ps, UNE_NT_NEG, &une_parse_negation);
  return une_parse_power(error, ps);
}

une_parser__(une_parse_power)
{
  LOGPARSE(L"", now(&ps->in));
  
  return une_parse_binary_operation(error, ps,
    UNE_TT_POW,
    UNE_NT_POW,
    UNE_TT_POW,
    &une_parse_accessor,
    &une_parse_power /* DOC: We parse power and not accessor because powers are evaluated right to left. */
  );
}

une_parser__(une_parse_accessor)
{
  LOGPARSE(L"", now(&ps->in));
  
  une_node *base = une_parse_atom(error, ps);
  if (base == NULL)
    return NULL;
  
  while (true) {
    une_node *accessor;
    if (now(&ps->in).type == UNE_TT_LSQB) {
      accessor = une_parse_index(error, ps);
      if (accessor)
        accessor->type = UNE_NT_IDX_SEEK;
    } else if (now(&ps->in).type == UNE_TT_LPAR) {
      accessor = une_parse_call(error, ps);
      if (accessor)
        accessor->type = UNE_NT_CALL;
    } else if (now(&ps->in).type == UNE_TT_DOT) {
      accessor = une_parse_member(error, ps);
      if (accessor)
        accessor->type = UNE_NT_MEMBER_SEEK;
    } else {
      break;
    }
    if (!accessor) {
      une_node_free(base, false);
      return NULL;
    }
    accessor->pos = une_position_set_start(accessor->pos, base->pos);
    accessor->content.branch.a = base;
    base = accessor;
  }
  
  return base;
}

une_parser__(une_parse_atom)
{
  LOGPARSE(L"", now(&ps->in));
  
  switch (now(&ps->in).type) {
    
    case UNE_TT_VOID:
      return une_parse_void(error, ps);
    
    case UNE_TT_INT:
      return une_parse_int(error, ps);
    
    case UNE_TT_FLT:
      return une_parse_flt(error, ps);
    
    case UNE_TT_STR:
      return une_parse_str(error, ps);
    
    case UNE_TT_TRUE:
      return une_parse_true(error, ps);
    
    case UNE_TT_FALSE:
      return une_parse_false(error, ps);
    
    case UNE_TT_THIS:
      return une_parse_this(error, ps);
    
    case UNE_TT_BUILTIN:
      return une_parse_builtin(error, ps);
    
    case UNE_TT_ID:
      return une_parse_seek(error, ps, true);
    
    case UNE_TT_LSQB:
      return une_parse_list(error, ps);
    
    case UNE_TT_LBRC:
      return une_parse_object(error, ps);

    case UNE_TT_FUNCTION:
      return une_parse_function(error, ps);
    
    case UNE_TT_LPAR: {
      LOGPARSE(L"expression", now(&ps->in));
      une_position pos_first = now(&ps->in).pos;
      pull(&ps->in);
      while (now(&ps->in).type == UNE_TT_NEW)
        pull(&ps->in);
      une_node *expression = une_parse_expression(error, ps);
      if (expression == NULL)
        return NULL;
      while (now(&ps->in).type == UNE_TT_NEW)
        pull(&ps->in);
      if (now(&ps->in).type != UNE_TT_RPAR) {
        une_node_free(expression, false);
        *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
        return NULL;
      }
      expression->pos = une_position_between(pos_first, now(&ps->in).pos);
      pull(&ps->in);
      return expression;
    }
    
    default:
      break;
  
  }
  
  *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
  return NULL;
}

une_parser__(une_parse_void)
{
  LOGPARSE(L"", now(&ps->in));
  une_node *void_ = une_node_create(UNE_NT_VOID);
  void_->pos = now(&ps->in).pos;
  void_->content.value._int = 0;
  pull(&ps->in);
  return void_;
}

une_parser__(une_parse_int)
{
  LOGPARSE(L"", now(&ps->in));
  une_node *num = une_node_create(UNE_NT_INT);
  num->pos = now(&ps->in).pos;
  num->content.value._int = now(&ps->in).value._int;
  pull(&ps->in);
  return num;
}

une_parser__(une_parse_flt)
{
  LOGPARSE(L"", now(&ps->in));
  une_node *num = une_node_create(UNE_NT_FLT);
  num->pos = now(&ps->in).pos;
  num->content.value._flt = now(&ps->in).value._flt;
  pull(&ps->in);
  return num;
}

une_parser__(une_parse_str)
{
  /* Guaranteed first string. */
  une_node *left = une_node_create(UNE_NT_STR);
  left->pos = now(&ps->in).pos;
  left->content.value._wcs = now(&ps->in).value._wcs;
  pull(&ps->in);
  
  while (now(&ps->in).type == UNE_TT_STR_EXPRESSION_BEGIN) {
    /* '{'. */
    pull(&ps->in);
    
    /* String expression. */
    une_node *expression = une_parse_expression(error, ps);
    if (!expression) {
      une_node_free(left, false);
      return NULL;
    }
    
    /* '}'. */
    if (now(&ps->in).type != UNE_TT_STR_EXPRESSION_END) {
      *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
      une_node_free(left, false);
      une_node_free(expression, false);
      return NULL;
    }
    pull(&ps->in);
    
    /* Next string. */
    assert(now(&ps->in).type == UNE_TT_STR);
    une_node *post_expression_string = une_node_create(UNE_NT_STR);
    post_expression_string->pos = now(&ps->in).pos;
    post_expression_string->content.value._wcs = now(&ps->in).value._wcs;
    pull(&ps->in);
    
    /* Combine nodes. */
    une_node *string_with_expression = une_node_create(UNE_NT_CONCATENATE);
    string_with_expression->pos = une_position_between(expression->pos, post_expression_string->pos);
    string_with_expression->pos = une_position_between(expression->pos, post_expression_string->pos);
    string_with_expression->content.branch.a = expression;
    string_with_expression->content.branch.b = post_expression_string;
    
    une_node *new_left = une_node_create(UNE_NT_CONCATENATE);
    new_left->content.branch.a = left;
    new_left->content.branch.b = string_with_expression;
    left = new_left;
  }
  
  return left;
}

une_parser__(une_parse_true)
{
  LOGPARSE(L"", now(&ps->in));
  une_node *num = une_node_create(UNE_NT_INT);
  num->pos = now(&ps->in).pos;
  num->content.value._int = 1;
  pull(&ps->in);
  return num;
}

une_parser__(une_parse_false)
{
  LOGPARSE(L"", now(&ps->in));
  une_node *num = une_node_create(UNE_NT_INT);
  num->pos = now(&ps->in).pos;
  num->content.value._int = 0;
  pull(&ps->in);
  return num;
}

une_parser__(une_parse_this)
{
  LOGPARSE(L"", now(&ps->in));
  une_node *num = une_node_create(UNE_NT_THIS);
  num->pos = now(&ps->in).pos;
  pull(&ps->in);
  return num;
}

une_parser__(une_parse_seek, bool global)
{
  LOGPARSE(L"", now(&ps->in));
  une_node *id = une_parse_id(error, ps);
  if (!id) {
    *error = une_error_create(); /* une_parse_id normally returns an error, but here we want to ignore it. */
    return NULL;
  }
  une_node *seek = une_node_create(UNE_NT_SEEK);
  seek->pos = id->pos;
  seek->content.branch.a = id;
  seek->content.branch.b = (une_node*)global;
  return seek;
}

une_parser__(une_parse_seek_or_this, bool global)
{
  LOGPARSE(L"", now(&ps->in));
  if (now(&ps->in).type == UNE_TT_THIS)
    return une_parse_this(error, ps);
  return une_parse_seek(error, ps, global);
}

une_parser__(une_parse_builtin)
{
  LOGPARSE(L"", now(&ps->in));
  if (!UNE_BUILTIN_FUNCTION_IS_VALID(now(&ps->in).value._int))
    return NULL;
  une_node *builtin = une_node_create(UNE_NT_BUILTIN);
  builtin->pos = now(&ps->in).pos;
  builtin->content.value._int = now(&ps->in).value._int;
  pull(&ps->in);
  return builtin;
}

une_parser__(une_parse_list)
{
  LOGPARSE(L"", now(&ps->in));
  
  return une_parse_sequence(error, ps, UNE_NT_LIST, UNE_TT_LSQB, UNE_TT_SEP, UNE_TT_RSQB, &une_parse_expression);
}

une_parser__(une_parse_function)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* function. */
  une_position pos_first = now(&ps->in).pos;
  void *definition_file = now(&ps->in).value._vp;
  pull(&ps->in);
  
  /* Parameters. */
  une_node *params = une_parse_sequence(error, ps, UNE_NT_LIST, UNE_TT_LPAR, UNE_TT_SEP, UNE_TT_RPAR, &une_parse_id);
  if (params == NULL)
    return NULL;
  
  /* Body. */
  une_node *body = une_parse_stmt(error, ps);
  if (body == NULL) {
    une_node_free(params, false);
    return NULL;
  }
  
  une_node *function = une_node_create(UNE_NT_FUNCTION);
  function->pos = une_position_between(pos_first, body->pos);
  function->content.branch.a = params;
  function->content.branch.b = body;
  function->content.branch.c = definition_file;
  return function;
}

une_parser__(une_parse_for)
{
  LOGPARSE(L"", now(&ps->in));
  
  une_position pos_first = now(&ps->in).pos;
  
  /* For. */
  pull(&ps->in);
  
  /* Id. */
  une_node *counter = une_parse_id(error, ps);
  if (counter == NULL)
    return NULL;
  
  /* Realm. */
  une_node *loop = NULL;
  if (now(&ps->in).type == UNE_TT_FROM)
    loop = une_parse_for_range(error, ps);
  else if (now(&ps->in).type == UNE_TT_IN)
    loop = une_parse_for_element(error, ps);
  else
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
  if (!loop) {
    une_node_free(counter, false);
    return NULL;
  }
  
  /* Body. */
  ps->loop_level++;
  une_node *body = une_parse_stmt(error, ps);
  ps->loop_level--;
  if (body == NULL) {
    une_node_free(counter, false);
    une_node_free(loop, false);
    return NULL;
  }
  
  loop->pos = une_position_between(pos_first, peek(&ps->in, -1).pos);
  loop->content.branch.a = counter;
  if (loop->type == UNE_NT_FOR_RANGE)
    loop->content.branch.d = body;
  else
    loop->content.branch.c = body;
  return loop;
}

une_parser__(une_parse_for_range)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* From. */
  assert(now(&ps->in).type == UNE_TT_FROM);
  pull(&ps->in);
  
  /* Expression. */
  une_node *from = une_parse_expression(error, ps);
  if (from == NULL)
    return NULL;
  
  /* Till. */
  if (now(&ps->in).type != UNE_TT_TILL) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    une_node_free(from, false);
    return NULL;
  }
  pull(&ps->in);
  
  /* Expression. */
  une_node *till = une_parse_expression(error, ps);
  if (till == NULL) {
    une_node_free(from, false);
    return NULL;
  }
  
  une_node *loop = une_node_create(UNE_NT_FOR_RANGE);
  loop->content.branch.b = from;
  loop->content.branch.c = till;
  return loop;
}

une_parser__(une_parse_for_element)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* In. */
  assert(now(&ps->in).type == UNE_TT_IN);
  pull(&ps->in);
  
  /* Expression. */
  une_node *elements = une_parse_expression(error, ps);
  if (elements == NULL)
    return NULL;
  
  une_node *loop = une_node_create(UNE_NT_FOR_ELEMENT);
  loop->content.branch.b = elements;
  return loop;
}

une_parser__(une_parse_while)
{
  LOGPARSE(L"", now(&ps->in));
  
  une_position pos_first = now(&ps->in).pos;
  
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
  node->pos = une_position_between(pos_first, body->pos);
  node->content.branch.a = condition;
  node->content.branch.b = body;
  return node;
}

une_parser__(une_parse_if)
{
  LOGPARSE(L"", now(&ps->in));
  
  une_position pos_first = now(&ps->in).pos;
  
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
  ptrdiff_t _token_index = ps->in.index; /* Here we skip over whitespace expecting
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
  ifstmt->pos = une_position_set_start(ifstmt->pos, pos_first);
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

une_parser__(une_parse_assert)
{
  LOGPARSE(L"", now(&ps->in));
  
  une_position pos_first = now(&ps->in).pos;
  
  /* Assert. */
  pull(&ps->in);

  /* Assertion. */
  une_node *assertion = une_parse_expression(error, ps);
  if (assertion == NULL)
    return NULL;

  une_node *assert_ = une_node_create(UNE_NT_ASSERT);
  assert_->pos = une_position_set_start(assert_->pos, pos_first);
  assert_->pos.end = assertion->pos.end;
  assert_->content.branch.a = assertion;
  return assert_;
}

une_parser__(une_parse_continue)
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

une_parser__(une_parse_break)
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

une_parser__(une_parse_return)
{
  LOGPARSE(L"", now(&ps->in));
  
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
      return NULL;
    pos.end = value->pos.end;
  }

  une_node *return_ = une_node_create(UNE_NT_RETURN);
  return_->pos = pos;
  return_->content.branch.a = value;
  return return_;
}

une_parser__(une_parse_exit)
{
  LOGPARSE(L"", now(&ps->in));
  
  une_position pos = now(&ps->in).pos; /* If the parser finds an exit
                                          code after this, pos.end is
                                          changed again further down. */
  pull(&ps->in); /* Exit. */

  /* Exit Code. */
  une_node *value = NULL; /* DOC: NULL here means no exit code was specified.
                             This tells the interpreter that there is no exit
                             code. */
  if (now(&ps->in).type != UNE_TT_NEW && now(&ps->in).type != UNE_TT_EOF && now(&ps->in).type != UNE_TT_RBRC) {
    value = une_parse_expression(error, ps);
    if (value == NULL) /* DOC: NULL here means an error. */
      return NULL;
    pos.end = value->pos.end;
  }

  une_node *exit = une_node_create(UNE_NT_EXIT);
  exit->pos = pos;
  exit->content.branch.a = value;
  return exit;
}

une_parser__(une_parse_assignment_or_expr_stmt)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* Check for variable assignment. */
  ptrdiff_t token_index_before = ps->in.index; /* Needed to backstep in case this is not a variable assignment. */
  une_node *assignee = une_parse_assignee(error, ps);
  une_node_type assignment_operation = UNE_NT_none__;
  if (
    assignee &&
    now(&ps->in).type >= UNE_R_BGN_ASSIGNMENT_TOKENS &&
    now(&ps->in).type <= UNE_R_END_ASSIGNMENT_TOKENS
  ) {
    assignment_operation = UNE_R_BGN_ASSIGNMENT_NODES + (now(&ps->in).type - UNE_R_BGN_ASSIGNMENT_TOKENS);
    pull(&ps->in);
  } else {
    ps->in.index = token_index_before;
  }
  
  LOGPARSE(L"expression", now(&ps->in));
  
  /* Expression. */
  une_node *expression = une_parse_expression(error, ps);
  if (!expression) {
    une_node_free(assignee, false);
    return NULL;
  }
  
  /* Resolve final node type. */
  une_node *assignment_or_expression_statement;
  if (assignment_operation) {
    assignment_or_expression_statement = une_node_create(assignment_operation);
    assignment_or_expression_statement->pos = une_position_between(assignee->pos, expression->pos);
    assignment_or_expression_statement->content.branch.a = assignee;
    assignment_or_expression_statement->content.branch.b = expression;
  } else {
    une_node_free(assignee, false);
    assignment_or_expression_statement = expression;
  }
  
  return assignment_or_expression_statement;
}

une_parser__(une_parse_assignee)
{
  LOGPARSE(L"", now(&ps->in));
  
  bool global = false;
  if (now(&ps->in).type == UNE_TT_GLOBAL) {
    pull(&ps->in);
    global = true;
  }
  
  une_node *base = une_parse_seek_or_this(error, ps, global);
  if (!base)
    return NULL;
  
  while (true) {
    une_node *accessor;
    if (now(&ps->in).type == UNE_TT_LSQB) {
      accessor = une_parse_index(error, ps);
      if (accessor)
        accessor->type = UNE_NT_IDX_SEEK;
    } else if (now(&ps->in).type == UNE_TT_DOT) {
      accessor = une_parse_member(error, ps);
      if (accessor)
        accessor->type = UNE_NT_MEMBER_SEEK;
    } else {
      break;
    }
    if (!accessor) {
      une_node_free(base, false);
      return NULL;
    }
    accessor->pos = une_position_set_start(accessor->pos, base->pos);
    accessor->content.branch.a = base;
    base = accessor;
  }
  
  return base;
}

une_parser__(une_parse_index)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* '['. */
  assert(now(&ps->in).type == UNE_TT_LSQB);
  pull(&ps->in);
  
  /* Begin? */
  une_node *begin = NULL;
  if (now(&ps->in).type == UNE_TT_DOTDOT) {
    begin = une_parse_phony(error, ps, UNE_NT_INT);
    begin->content.value._int = 0;
  } else {
    begin = une_parse_expression(error, ps);
  }
  if (!begin)
    return NULL;
  
  /* Range? */
  une_node *end = NULL;
  if (now(&ps->in).type == UNE_TT_DOTDOT) {
    pull(&ps->in); /* '..'. */
    /* End? */
    if (now(&ps->in).type == UNE_TT_RSQB)
      end = une_parse_phony(error, ps, UNE_NT_VOID);
    else
      end = une_parse_expression(error, ps);
    if (!end) {
      une_node_free(begin, false);
      return NULL;
    }
  }
  
  /* ']'. */
  if (now(&ps->in).type != UNE_TT_RSQB) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    une_node_free(begin, false);
    une_node_free(end, false);
    return NULL;
  }
  pull(&ps->in);
  
  une_node *index = une_node_create(UNE_NT_none__); /* The caller is required to provide the node type. */
  index->pos.end = peek(&ps->in, -1).pos.end; /* The caller is required to provide the start position. */
  index->content.branch.b = begin; /* The caller is required to provide branch A. */
  index->content.branch.c = end;
  return index;
}

une_parser__(une_parse_call)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* Arguments. */
  une_node *arguments = une_parse_sequence(error, ps, UNE_NT_LIST, UNE_TT_LPAR, UNE_TT_SEP, UNE_TT_RPAR, &une_parse_expression);
  if (!arguments)
    return NULL;
  
  une_node *call = une_node_create(UNE_NT_none__); /* The caller is required to provide the node type. */
  call->pos.end = arguments->pos.end; /* The caller is required to provide the start position. */
  call->content.branch.b = arguments; /* The caller is required to provide branch A. */
  return call;
}

une_parser__(une_parse_member)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* '.'. */
  assert(now(&ps->in).type == UNE_TT_DOT);
  pull(&ps->in);
  
  /* Identifier. */
  une_node *identifier = une_parse_id(error, ps);
  if (!identifier)
    return NULL;
  
  une_node *member = une_node_create(UNE_NT_none__); /* The caller is required to provide the node type. */
  member->pos.end = identifier->pos.end; /* The caller is required to provide the start position. */
  member->content.branch.b = identifier; /* The caller is required to provide branch A. */
  return member;
}

une_parser__(une_parse_object_association)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* Name. */
  une_node *name = une_parse_id(error, ps);
  if (!name)
    return NULL;
  
  /* ':'. */
  if (now(&ps->in).type != UNE_TT_COLON) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    une_node_free(name, false);
    return NULL;
  }
  pull(&ps->in);
  
  /* Expression. */
  une_node *expression = une_parse_expression(error, ps);
  if (!expression) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    une_node_free(name, false);
    return NULL;
  }
  
  une_node *object_association = une_node_create(UNE_NT_OBJECT_ASSOCIATION);
  object_association->pos = une_position_between(name->pos, expression->pos);
  object_association->content.branch.a = name;
  object_association->content.branch.b = expression;
  return object_association;
}

une_parser__(une_parse_object)
{
  LOGPARSE(L"", now(&ps->in));
  
  return une_parse_sequence(error, ps, UNE_NT_OBJECT, UNE_TT_LBRC, UNE_TT_SEP, UNE_TT_RBRC, &une_parse_object_association);
}

une_parser__(une_parse_unary_operation, une_node_type node_t, une_node *(*parse)(une_error*, une_parser_state*)
)
{
  LOGPARSE(L"", now(&ps->in));
  
  une_position pos_first = now(&ps->in).pos;
  
  pull(&ps->in);
  
  une_node *node = (*parse)(error, ps);
  if (node == NULL)
    return NULL;
  
  une_node *unop = une_node_create(node_t);
  unop->pos = une_position_between(pos_first, node->pos);
  unop->content.branch.a = node;
  return unop;
}

une_parser__(une_parse_binary_operation,
  une_token_type range_begin_tt,
  une_node_type range_begin_nt,
  une_token_type range_end_tt,
  une_node *(*parse_left)(une_error*, une_parser_state*),
  une_node *(*parse_right)(une_error*, une_parser_state*)
)
{
  LOGPARSE(L"", now(&ps->in));
  
  une_node *left = (*parse_left)(error, ps);
  if (left == NULL)
    return NULL;

  while (now(&ps->in).type >= range_begin_tt && now(&ps->in).type <= range_end_tt)
  {
    une_node_type type = range_begin_nt+(une_node_type)(now(&ps->in).type-range_begin_tt);

    pull(&ps->in);

    une_node *right = (*parse_right)(error, ps);
    if (right == NULL) {
      une_node_free(left, false);
      return NULL;
    }

    une_node *new_left = une_node_create(type);
    new_left->pos = une_position_between(left->pos, right->pos);
    new_left->content.branch.a = left;
    new_left->content.branch.b = right;
    left = new_left;
  }
  
  return left;
}

une_parser__(une_parse_sequence,
  une_node_type node_type,
  une_token_type tt_begin, une_token_type tt_end_of_item, une_token_type tt_end,
  une_node *(*parser)(une_error*, une_parser_state*)
)
{
  LOGPARSE(L"", now(&ps->in));
  
  /* Begin Sequence. */
  une_position pos_first = now(&ps->in).pos;
  if (tt_begin != UNE_TT_none__) {
    if (now(&ps->in).type != tt_begin) {
      *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
      return NULL;
    }
    pull(&ps->in);
  }
  
  /* Sequence. */
  size_t sequence_size = UNE_SIZE_SEQUENCE;
  une_node **sequence = malloc(sequence_size*sizeof(*sequence));
  verify(sequence);
  size_t sequence_index = 1;

  while (true) {
    if (sequence_index >= sequence_size) {
      sequence_size *= 2;
      sequence = realloc(sequence, sequence_size*sizeof(sequence));
      verify(sequence);
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
    
    /* ADDITIONAL WHITESPACE. */
    if (tt_end_of_item != UNE_TT_NEW)
      while (now(&ps->in).type == UNE_TT_NEW)
        pull(&ps->in);
    
    /* ITEM DELIMITER. */
    if (now(&ps->in).type == tt_end || now(&ps->in).type == tt_end_of_item)
      continue;
    for (size_t i=1; i<sequence_index; i++)
      une_node_free(sequence[i], false);
    free(sequence);
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, now(&ps->in).pos);
    return NULL;
  }

  /* CREATE NODE. */
  une_node *counter = une_node_create(UNE_NT_SIZE);
  counter->content.value._int = (une_int)sequence_index-1;
  sequence[0] = counter;
  une_node *node = une_node_create(node_type);
  node->pos = une_position_between(pos_first, now(&ps->in).pos);
  node->content.value._vpp = (void**)sequence;
  
  /* End Sequence. */
  /* We don't skip EOF because it may still be needed by other functions up
  the call chain. */
  if (tt_end != UNE_TT_EOF)
    pull(&ps->in);
  return node;
}

une_parser__(une_parse_phony,
  une_node_type node_type
)
{
  une_node *phony = une_node_create(node_type);
  phony->pos = une_position_set_start(phony->pos, now(&ps->in).pos);
  phony->pos.end = phony->pos.start;
  return phony;
}
