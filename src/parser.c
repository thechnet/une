/*
parser.c - Une
Modified 2024-11-09
*/

/* Header-specific includes. */
#include "parser.h"

/* Implementation-specific includes. */
#include "tools.h"
#include "deprecated/stream.h"
#include "natives.h"

#if defined(UNE_DEBUG) && defined(UNE_DBG_LOG_PARSE)
static int une_logparse_indent = 0;
#endif

/*
Public parser interface.
*/

UNE_ISTREAM_ARRAY_PULLER_VAL(pull, une_token, une_token, une_token_create(UNE_TK_none__), false)
UNE_ISTREAM_ARRAY_PEEKER_VAL(peek, une_token, une_token, une_token_create(UNE_TK_none__), false)
UNE_ISTREAM_ARRAY_ACCESS_VAL(now, une_token, une_token, une_token_create(UNE_TK_none__), false)

une_node *une_parse(une_error *error, une_parser_state *ps, une_token *tokens)
{
	/* Initialize une_parser_state. */
	ps->in = une_istream_array_create((void*)tokens, 0);
	pull(&ps->in);

	return une_parse_sequence(error, ps, UNE_NK_STMTS, UNE_TK_none__, UNE_TK_NEW, UNE_TK_EOF, &une_parse_directive_or_block);
}

une_parser__(une_parse_body)
{
	LOGPARSE_BEGIN();

	if (now(&ps->in).kind == UNE_TK_LBRC)
		LOGPARSE_END(une_parse_block(error, ps));
	else
		LOGPARSE_END(une_parse_sequence(error, ps, UNE_NK_STMTS, UNE_TK_none__, UNE_TK_none__, UNE_TK_none__, &une_parse_directive_or_block));
}

une_parser__(une_parse_directive_or_block)
{
	LOGPARSE_BEGIN();
	
	/* Skip NEW before/between statements. */
	une_parser_skip_whitespace(ps);
	
	switch (now(&ps->in).kind) {
		case UNE_TK_LBRC:
			LOGPARSE_END(une_parse_block(error, ps));
		case UNE_TK_FOR:
			LOGPARSE_END(une_parse_for(error, ps));
		case UNE_TK_WHILE:
			LOGPARSE_END(une_parse_while(error, ps));
		case UNE_TK_IF:
			LOGPARSE_END(une_parse_if(error, ps));
		case UNE_TK_ASSERT:
			LOGPARSE_END(une_parse_assert(error, ps));
		case UNE_TK_CONTINUE:
			LOGPARSE_END(une_parse_continue(error, ps));
		case UNE_TK_BREAK:
			LOGPARSE_END(une_parse_break(error, ps));
		case UNE_TK_RETURN:
			LOGPARSE_END(une_parse_return(error, ps));
		case UNE_TK_EXIT:
			LOGPARSE_END(une_parse_exit(error, ps));
		default:
			break;
	}
	
	LOGPARSE_END(une_parse_assignment_or_expr_stmt(error, ps));
}

une_parser__(une_parse_name)
{
	LOGPARSE_BEGIN();
	
	if (now(&ps->in).kind != UNE_TK_NAME) {
		*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
		LOGPARSE_END(NULL);
	}

	une_node *name = une_node_create(UNE_NK_NAME);
	name->pos = now(&ps->in).pos;
	name->content.value._wcs = now(&ps->in).value._wcs;
	pull(&ps->in);
	
	LOGPARSE_END(name);
}

une_parser__(une_parse_block)
{
	LOGPARSE_END(une_parse_sequence(error, ps, UNE_NK_STMTS, UNE_TK_LBRC, UNE_TK_NEW, UNE_TK_RBRC, &une_parse_directive_or_block));
}

une_parser__(une_parse_expression)
{
	LOGPARSE_BEGIN();

	LOGPARSE_END(une_parse_conditional(error, ps));
}

une_parser__(une_parse_conditional)
{
	LOGPARSE_BEGIN();
	
	/* Condition. */
	une_node *cond = une_parse_and_or(error, ps);
	if (!cond || now(&ps->in).kind != UNE_TK_QMARK)
		LOGPARSE_END(cond);

	/* ?. */
	pull(&ps->in);

	une_parser_skip_whitespace(ps);
	
	/* Expression. */
	une_node *exp_true = une_parse_and_or(error, ps);
	if (exp_true == NULL) {
		une_node_free(cond, false);
		LOGPARSE_END(NULL);
	}
	
	/* :. */
	if (now(&ps->in).kind != UNE_TK_COLON) {
		*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
		une_node_free(cond, false);
		une_node_free(exp_true, false);
		LOGPARSE_END(NULL);
	}
	pull(&ps->in);

	une_parser_skip_whitespace(ps);
	
	/* Expression. */
	une_node *exp_false = une_parse_expression(error, ps);
	if (exp_false == NULL) {
		une_node_free(exp_true, false);
		une_node_free(cond, false);
		LOGPARSE_END(NULL);
	}
	
	une_node *cop = une_node_create(UNE_NK_COP);
	cop->pos = une_position_between(cond->pos, exp_false->pos);
	cop->content.branch.a = cond;
	cop->content.branch.b = exp_true;
	cop->content.branch.c = exp_false;
	LOGPARSE_END(cop);
}

une_parser__(une_parse_and_or)
{
	LOGPARSE_BEGIN();
	
	LOGPARSE_END(une_parse_binary_operation(error, ps,
		UNE_R_BGN_AND_OR_TOKENS,
		UNE_R_BGN_AND_OR_NODES,
		UNE_R_END_AND_OR_TOKENS,
		&une_parse_condition,
		&une_parse_condition
	));
}

une_parser__(une_parse_condition)
{
	LOGPARSE_BEGIN();
	
	LOGPARSE_END(une_parse_binary_operation(error, ps,
		UNE_R_BGN_CONDITION_TOKENS,
		UNE_R_BGN_CONDITION_NODES,
		UNE_R_END_CONDITION_TOKENS,
		&une_parse_any_all,
		&une_parse_any_all
	));
}

une_parser__(une_parse_any_all)
{
	LOGPARSE_BEGIN();
	
	if (now(&ps->in).kind == UNE_TK_ANY)
		LOGPARSE_END(une_parse_unary_operation(error, ps, UNE_NK_ANY, &une_parse_cover));
	if (now(&ps->in).kind == UNE_TK_ALL)
		LOGPARSE_END(une_parse_unary_operation(error, ps, UNE_NK_ALL, &une_parse_cover));
	LOGPARSE_END(une_parse_cover(error, ps));
}

une_parser__(une_parse_cover)
{
	LOGPARSE_BEGIN();
	
	LOGPARSE_END(une_parse_binary_operation(error, ps,
		UNE_TK_COVER,
		UNE_NK_COVER,
		UNE_TK_COVER,
		&une_parse_add_sub,
		&une_parse_add_sub
	));
}

une_parser__(une_parse_add_sub)
{
	LOGPARSE_BEGIN();
	
	LOGPARSE_END(une_parse_binary_operation(error, ps,
		UNE_R_BGN_ADD_SUB_TOKENS,
		UNE_R_BGN_ADD_SUB_NODES,
		UNE_R_END_ADD_SUB_TOKENS,
		&une_parse_term,
		&une_parse_term
	));
}

une_parser__(une_parse_term)
{
	LOGPARSE_BEGIN();
	
	LOGPARSE_END(une_parse_binary_operation(error, ps,
		UNE_R_BGN_TERM_TOKENS,
		UNE_R_BEGIN_TERM_NODES,
		UNE_R_END_TERM_TOKENS,
		&une_parse_negation,
		&une_parse_negation
	));
}

une_parser__(une_parse_negation)
{
	LOGPARSE_BEGIN();
	
	if (now(&ps->in).kind == UNE_TK_NOT)
		LOGPARSE_END(une_parse_unary_operation(error, ps, UNE_NK_NOT, &une_parse_negation));
	LOGPARSE_END(une_parse_minus(error, ps));
}

une_parser__(une_parse_minus)
{
	LOGPARSE_BEGIN();
	
	if (now(&ps->in).kind == UNE_TK_SUB)
		LOGPARSE_END(une_parse_unary_operation(error, ps, UNE_NK_NEG, &une_parse_minus));
	LOGPARSE_END(une_parse_power(error, ps));
}

une_parser__(une_parse_power)
{
	LOGPARSE_BEGIN();
	
	LOGPARSE_END(une_parse_binary_operation(error, ps,
		UNE_TK_POW,
		UNE_NK_POW,
		UNE_TK_POW,
		&une_parse_accessor,
		&une_parse_minus /* We parse a lower precedence here because powers are evaluated right to left. */
	));
}

une_parser__(une_parse_accessor)
{
	LOGPARSE_BEGIN();
	
	une_node *base = une_parse_atom(error, ps);
	if (base == NULL)
		LOGPARSE_END(NULL);
	
	while (true) {
		une_node *accessor;
		if (now(&ps->in).kind == UNE_TK_LSQB) {
			accessor = une_parse_index(error, ps);
			if (accessor)
				accessor->kind = UNE_NK_IDX_SEEK;
		} else if (now(&ps->in).kind == UNE_TK_LPAR) {
			accessor = une_parse_call(error, ps);
			if (accessor)
				accessor->kind = UNE_NK_CALL;
		} else if (now(&ps->in).kind == UNE_TK_DOT) {
			accessor = une_parse_member(error, ps);
			if (accessor)
				accessor->kind = UNE_NK_MEMBER_SEEK;
		} else {
			break;
		}
		if (!accessor) {
			une_node_free(base, false);
			LOGPARSE_END(NULL);
		}
		accessor->pos = une_position_set_start(accessor->pos, base->pos);
		accessor->content.branch.a = base;
		base = accessor;
	}
	
	LOGPARSE_END(base);
}

une_parser__(une_parse_atom)
{
	LOGPARSE_BEGIN();
	
	switch (now(&ps->in).kind) {
		
		case UNE_TK_VOID:
			LOGPARSE_END(une_parse_void(error, ps));
		
		case UNE_TK_INT:
			LOGPARSE_END(une_parse_int(error, ps));
		
		case UNE_TK_FLT:
			LOGPARSE_END(une_parse_flt(error, ps));
		
		case UNE_TK_STR:
			LOGPARSE_END(une_parse_str(error, ps));
		
		case UNE_TK_TRUE:
			LOGPARSE_END(une_parse_true(error, ps));
		
		case UNE_TK_FALSE:
			LOGPARSE_END(une_parse_false(error, ps));
		
		case UNE_TK_THIS:
			LOGPARSE_END(une_parse_this(error, ps));
		
		case UNE_TK_NATIVE:
			LOGPARSE_END(une_parse_native(error, ps));
		
		case UNE_TK_LSQB:
			LOGPARSE_END(une_parse_list(error, ps));
		
		case UNE_TK_LBRC:
			LOGPARSE_END(une_parse_object(error, ps));

		case UNE_TK_NAME: {
			ptrdiff_t before_signature = une_parser_checkpoint(ps);
			une_node *parameters = une_parse_signature(error, ps);
			if (parameters) {
				LOGPARSE_COMMENT(L"signature exists -> function with non-parenthesized parameter");
				LOGPARSE_END(une_parse_function(error, ps, parameters));
			}
			une_parser_return_to_checkpoint(error, ps, before_signature);
			LOGPARSE_COMMENT(L"no signature -> variable name");
			LOGPARSE_END(une_parse_seek(error, ps, true));
		}
		
		case UNE_TK_LPAR: {
			ptrdiff_t before_signature = une_parser_checkpoint(ps);
			une_node *parameters = une_parse_signature(error, ps);
			if (parameters) {
				LOGPARSE_COMMENT(L"signature exists -> function with parenthesized parameters");
				LOGPARSE_END(une_parse_function(error, ps, parameters));
			}
			une_parser_return_to_checkpoint(error, ps, before_signature);
			LOGPARSE_COMMENT(L"no signature -> parenthesized expression");
			une_position pos_first = now(&ps->in).pos;
			pull(&ps->in);
			une_parser_skip_whitespace(ps);
			une_node *expression = une_parse_expression(error, ps);
			if (expression == NULL)
				LOGPARSE_END(NULL);
			une_parser_skip_whitespace(ps);
			if (now(&ps->in).kind != UNE_TK_RPAR) {
				une_node_free(expression, false);
				*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
				LOGPARSE_END(NULL);
			}
			expression->pos = une_position_between(pos_first, now(&ps->in).pos);
			pull(&ps->in);
			LOGPARSE_END(expression);
		}
		
		default:
			break;
	
	}
	
	*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
	LOGPARSE_END(NULL);
}

une_parser__(une_parse_void)
{
	LOGPARSE_BEGIN();
	une_node *void_ = une_node_create(UNE_NK_VOID);
	void_->pos = now(&ps->in).pos;
	void_->content.value._int = 0;
	pull(&ps->in);
	LOGPARSE_END(void_);
}

une_parser__(une_parse_int)
{
	LOGPARSE_BEGIN();
	une_node *num = une_node_create(UNE_NK_INT);
	num->pos = now(&ps->in).pos;
	num->content.value._int = now(&ps->in).value._int;
	pull(&ps->in);
	LOGPARSE_END(num);
}

une_parser__(une_parse_flt)
{
	LOGPARSE_BEGIN();
	une_node *num = une_node_create(UNE_NK_FLT);
	num->pos = now(&ps->in).pos;
	num->content.value._flt = now(&ps->in).value._flt;
	pull(&ps->in);
	LOGPARSE_END(num);
}

une_parser__(une_parse_str)
{
	/* Guaranteed first string. */
	une_node *left = une_node_create(UNE_NK_STR);
	left->pos = now(&ps->in).pos;
	left->content.value._wcs = now(&ps->in).value._wcs;
	pull(&ps->in);
	
	while (now(&ps->in).kind == UNE_TK_STR_EXPRESSION_BEGIN) {
		/* '{'. */
		pull(&ps->in);
		
		/* String expression. */
		une_node *expression = une_parse_expression(error, ps);
		if (!expression) {
			une_node_free(left, false);
			LOGPARSE_END(NULL);
		}
		
		/* '}'. */
		if (now(&ps->in).kind != UNE_TK_STR_EXPRESSION_END) {
			*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
			une_node_free(left, false);
			une_node_free(expression, false);
			LOGPARSE_END(NULL);
		}
		pull(&ps->in);
		
		/* Next string. */
		assert(now(&ps->in).kind == UNE_TK_STR);
		une_node *post_expression_string = une_node_create(UNE_NK_STR);
		post_expression_string->pos = now(&ps->in).pos;
		post_expression_string->content.value._wcs = now(&ps->in).value._wcs;
		pull(&ps->in);
		
		/* Combine nodes. */
		une_node *string_with_expression = une_node_create(UNE_NK_CONCATENATE);
		string_with_expression->pos = une_position_between(expression->pos, post_expression_string->pos);
		string_with_expression->pos = une_position_between(expression->pos, post_expression_string->pos);
		string_with_expression->content.branch.a = expression;
		string_with_expression->content.branch.b = post_expression_string;
		
		une_node *new_left = une_node_create(UNE_NK_CONCATENATE);
		new_left->content.branch.a = left;
		new_left->content.branch.b = string_with_expression;
		left = new_left;
	}
	
	LOGPARSE_END(left);
}

une_parser__(une_parse_true)
{
	LOGPARSE_BEGIN();
	une_node *num = une_node_create(UNE_NK_INT);
	num->pos = now(&ps->in).pos;
	num->content.value._int = 1;
	pull(&ps->in);
	LOGPARSE_END(num);
}

une_parser__(une_parse_false)
{
	LOGPARSE_BEGIN();
	une_node *num = une_node_create(UNE_NK_INT);
	num->pos = now(&ps->in).pos;
	num->content.value._int = 0;
	pull(&ps->in);
	LOGPARSE_END(num);
}

une_parser__(une_parse_this)
{
	LOGPARSE_BEGIN();
	une_node *num = une_node_create(UNE_NK_THIS);
	num->pos = now(&ps->in).pos;
	pull(&ps->in);
	LOGPARSE_END(num);
}

une_parser__(une_parse_seek, bool global)
{
	LOGPARSE_BEGIN();
	une_node *name = une_parse_name(error, ps);
	if (!name) {
		*error = une_error_create(); /* une_parse_name normally returns an error, but here we want to ignore it. */
		LOGPARSE_END(NULL);
	}
	une_node *seek = une_node_create(UNE_NK_SEEK);
	seek->pos = name->pos;
	seek->content.branch.a = name;
	seek->content.branch.b = (une_node*)global;
	LOGPARSE_END(seek);
}

une_parser__(une_parse_seek_or_this, bool global)
{
	LOGPARSE_BEGIN();
	if (now(&ps->in).kind == UNE_TK_THIS)
		LOGPARSE_END(une_parse_this(error, ps));
	LOGPARSE_END(une_parse_seek(error, ps, global));
}

une_parser__(une_parse_native)
{
	LOGPARSE_BEGIN();
	if (!UNE_NATIVE_IS_VALID(now(&ps->in).value._int))
		LOGPARSE_END(NULL);
	une_node *native = une_node_create(UNE_NK_NATIVE);
	native->pos = now(&ps->in).pos;
	native->content.value._int = now(&ps->in).value._int;
	pull(&ps->in);
	LOGPARSE_END(native);
}

une_parser__(une_parse_list)
{
	LOGPARSE_BEGIN();
	
	LOGPARSE_END(une_parse_sequence(error, ps, UNE_NK_LIST, UNE_TK_LSQB, UNE_TK_SEP, UNE_TK_RSQB, &une_parse_expression));
}

une_parser__(une_parse_signature)
{
	LOGPARSE_BEGIN();
	
	/* Parameters. */
	une_node *parameters = NULL;
	if (now(&ps->in).kind == UNE_TK_LPAR)
		parameters = une_parse_sequence(error, ps, UNE_NK_LIST, UNE_TK_LPAR, UNE_TK_SEP, UNE_TK_RPAR, &une_parse_name);
	else
		parameters = une_parse_sequence(error, ps, UNE_NK_LIST, UNE_TK_none__, UNE_TK_none__, UNE_TK_none__, &une_parse_name);
	if (!parameters)
		LOGPARSE_END(NULL);
	
	LOGPARSE_COMMENT(L"parameters correct, expecting '->'");

	/* '->'. */
	if (now(&ps->in).kind != UNE_TK_RIGHTARROW) {
		*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
		une_node_free(parameters, false);
		LOGPARSE_END(NULL);
	}
	pull(&ps->in);

	LOGPARSE_END(parameters);
}

une_parser__(une_parse_function, une_node *parameters)
{
	LOGPARSE_BEGIN();

	assert(parameters);
	
	/* Body. */
	une_node *body = une_parse_body(error, ps);
	if (body == NULL) {
		une_node_free(parameters, false);
		LOGPARSE_END(NULL);
	}
	
	une_node *module_id = une_node_create(UNE_NK_ID);
	module_id->content.value._id = ps->module_id;
	
	une_node *function = une_node_create(UNE_NK_FUNCTION);
	function->pos = une_position_between(parameters->pos, body->pos);
	function->content.branch.a = parameters;
	function->content.branch.b = body;
	function->content.branch.c = module_id;
	
	LOGPARSE_END(function);
}

une_parser__(une_parse_for)
{
	LOGPARSE_BEGIN();
	
	une_position pos_first = now(&ps->in).pos;
	
	/* For. */
	pull(&ps->in);
	
	/* Id. */
	une_node *counter = une_parse_name(error, ps);
	if (counter == NULL)
		LOGPARSE_END(NULL);
	
	/* Realm. */
	une_node *loop = NULL;
	if (now(&ps->in).kind == UNE_TK_FROM)
		loop = une_parse_for_range(error, ps);
	else if (now(&ps->in).kind == UNE_TK_IN)
		loop = une_parse_for_element(error, ps);
	else
		*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
	if (!loop) {
		une_node_free(counter, false);
		LOGPARSE_END(NULL);
	}
	
	/* Body. */
	ps->loop_level++;
	une_node *body = une_parse_body(error, ps);
	ps->loop_level--;
	if (body == NULL) {
		une_node_free(counter, false);
		une_node_free(loop, false);
		LOGPARSE_END(NULL);
	}
	
	loop->pos = une_position_between(pos_first, peek(&ps->in, -1).pos);
	loop->content.branch.a = counter;
	if (loop->kind == UNE_NK_FOR_RANGE)
		loop->content.branch.d = body;
	else
		loop->content.branch.c = body;
	LOGPARSE_END(loop);
}

une_parser__(une_parse_for_range)
{
	LOGPARSE_BEGIN();
	
	/* From. */
	assert(now(&ps->in).kind == UNE_TK_FROM);
	pull(&ps->in);
	
	/* Expression. */
	une_node *from = une_parse_expression(error, ps);
	if (from == NULL)
		LOGPARSE_END(NULL);
	
	/* Till. */
	if (now(&ps->in).kind != UNE_TK_TILL) {
		*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
		une_node_free(from, false);
		LOGPARSE_END(NULL);
	}
	pull(&ps->in);
	
	/* Expression. */
	une_node *till = une_parse_expression(error, ps);
	if (till == NULL) {
		une_node_free(from, false);
		LOGPARSE_END(NULL);
	}
	
	une_node *loop = une_node_create(UNE_NK_FOR_RANGE);
	loop->content.branch.b = from;
	loop->content.branch.c = till;
	LOGPARSE_END(loop);
}

une_parser__(une_parse_for_element)
{
	LOGPARSE_BEGIN();
	
	/* In. */
	assert(now(&ps->in).kind == UNE_TK_IN);
	pull(&ps->in);
	
	/* Expression. */
	une_node *elements = une_parse_expression(error, ps);
	if (elements == NULL)
		LOGPARSE_END(NULL);
	
	une_node *loop = une_node_create(UNE_NK_FOR_ELEMENT);
	loop->content.branch.b = elements;
	LOGPARSE_END(loop);
}

une_parser__(une_parse_while)
{
	LOGPARSE_BEGIN();
	
	une_position pos_first = now(&ps->in).pos;
	
	/* While. */
	pull(&ps->in);
	
	/* Condition. */
	une_node *condition = une_parse_expression(error, ps);
	if (condition == NULL)
		LOGPARSE_END(NULL);
	
	/* Body. */
	ps->loop_level++;
	une_node *body = une_parse_body(error, ps);
	ps->loop_level--;
	if (body == NULL) {
		une_node_free(condition, false);
		LOGPARSE_END(NULL);
	}
	
	une_node *node = une_node_create(UNE_NK_WHILE);
	node->pos = une_position_between(pos_first, body->pos);
	node->content.branch.a = condition;
	node->content.branch.b = body;
	LOGPARSE_END(node);
}

une_parser__(une_parse_if)
{
	LOGPARSE_BEGIN();
	
	une_position pos_first = now(&ps->in).pos;
	
	/* If || Elif. */
	pull(&ps->in);

	/* Condition. */
	une_node *predicate = une_parse_expression(error, ps);
	if (predicate == NULL)
		LOGPARSE_END(NULL);
	
	/* Body. */
	une_node *consequent = une_parse_body(error, ps);
	if (consequent == NULL) {
		une_node_free(predicate, false);
		LOGPARSE_END(NULL);
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
	une_parser_skip_whitespace(ps);

	une_node *ifstmt = une_node_create(UNE_NK_IF);
	ifstmt->pos = une_position_set_start(ifstmt->pos, pos_first);
	ifstmt->content.branch.a = predicate;
	ifstmt->content.branch.b = consequent;

	/* Only If Body. */
	if (now(&ps->in).kind != UNE_TK_ELSE && now(&ps->in).kind != UNE_TK_ELIF) {
		ps->in.index = _token_index;
		ifstmt->pos.end = consequent->pos.end;
		LOGPARSE_END(ifstmt);
	}
	
	/* Elif Body || Else Body. */
	une_node *alternate;
	if (now(&ps->in).kind == UNE_TK_ELIF)
		alternate = une_parse_if(error, ps);
	else {
		pull(&ps->in);
		alternate = une_parse_body(error, ps);
	}
	if (alternate == NULL) {
		une_node_free(ifstmt, false);
		LOGPARSE_END(NULL);
	}
	ifstmt->pos.end = alternate->pos.end;
	ifstmt->content.branch.c = alternate;
	LOGPARSE_END(ifstmt);
}

une_parser__(une_parse_assert)
{
	LOGPARSE_BEGIN();
	
	une_position pos_first = now(&ps->in).pos;
	
	/* Assert. */
	pull(&ps->in);

	/* Assertion. */
	une_node *assertion = une_parse_expression(error, ps);
	if (assertion == NULL)
		LOGPARSE_END(NULL);

	une_node *assert_ = une_node_create(UNE_NK_ASSERT);
	assert_->pos = une_position_set_start(assert_->pos, pos_first);
	assert_->pos.end = assertion->pos.end;
	assert_->content.branch.a = assertion;
	LOGPARSE_END(assert_);
}

une_parser__(une_parse_continue)
{
	if (ps->loop_level == 0) {
		*error = UNE_ERROR_SET(UNE_EK_CONTINUE_OUTSIDE_LOOP, now(&ps->in).pos);
		LOGPARSE_END(NULL);
	}
	une_node *continue_ = une_node_create(UNE_NK_CONTINUE);
	continue_->pos = now(&ps->in).pos;
	pull(&ps->in);
	LOGPARSE_END(continue_);
}

une_parser__(une_parse_break)
{
	if (ps->loop_level == 0) {
		*error = UNE_ERROR_SET(UNE_EK_BREAK_OUTSIDE_LOOP, now(&ps->in).pos);
		LOGPARSE_END(NULL);
	}
	une_node *break_ = une_node_create(UNE_NK_BREAK);
	break_->pos = now(&ps->in).pos;
	pull(&ps->in);
	LOGPARSE_END(break_);
}

une_parser__(une_parse_return)
{
	LOGPARSE_BEGIN();
	
	une_position pos = now(&ps->in).pos; /* If the parser finds a return
																						value after this, pos.end is
																						changed again further down. */
	pull(&ps->in); /* Return. */

	/* Return Value. */
	une_node *value = NULL; /* DOC: NULL here means no return value was specified.
														 This tells the interpreter that there is no return
														 value. */
	if (now(&ps->in).kind != UNE_TK_NEW && now(&ps->in).kind != UNE_TK_EOF && now(&ps->in).kind != UNE_TK_RBRC) {
		value = une_parse_expression(error, ps);
		if (value == NULL) /* DOC: NULL here means an error. */
			LOGPARSE_END(NULL);
		pos.end = value->pos.end;
	}

	une_node *return_ = une_node_create(UNE_NK_RETURN);
	return_->pos = pos;
	return_->content.branch.a = value;
	LOGPARSE_END(return_);
}

une_parser__(une_parse_exit)
{
	LOGPARSE_BEGIN();
	
	une_position pos = now(&ps->in).pos; /* If the parser finds an exit
																					code after this, pos.end is
																					changed again further down. */
	pull(&ps->in); /* Exit. */

	/* Exit Code. */
	une_node *value = NULL; /* DOC: NULL here means no exit code was specified.
														 This tells the interpreter that there is no exit
														 code. */
	if (now(&ps->in).kind != UNE_TK_NEW && now(&ps->in).kind != UNE_TK_EOF && now(&ps->in).kind != UNE_TK_RBRC) {
		value = une_parse_expression(error, ps);
		if (value == NULL) /* DOC: NULL here means an error. */
			LOGPARSE_END(NULL);
		pos.end = value->pos.end;
	}

	une_node *exit = une_node_create(UNE_NK_EXIT);
	exit->pos = pos;
	exit->content.branch.a = value;
	LOGPARSE_END(exit);
}

une_parser__(une_parse_assignment_or_expr_stmt)
{
	LOGPARSE_BEGIN();
	
	/* Check for variable assignment. */
	ptrdiff_t before_assignee = une_parser_checkpoint(ps); /* Needed to backstep in case this is not a variable assignment. */
	une_node *assignee = une_parse_assignee(error, ps);
	une_node_kind assignment_operation = UNE_NK_none__;
	if (
		assignee &&
		now(&ps->in).kind >= UNE_R_BGN_ASSIGNMENT_TOKENS &&
		now(&ps->in).kind <= UNE_R_END_ASSIGNMENT_TOKENS
	) {
		assignment_operation = UNE_R_BGN_ASSIGNMENT_NODES + (now(&ps->in).kind - UNE_R_BGN_ASSIGNMENT_TOKENS);
		pull(&ps->in);
	} else {
		une_parser_return_to_checkpoint(error, ps, before_assignee);
	}
	
	LOGPARSE_COMMENT(L"no variable assignment -> expression statement");
	
	/* Expression. */
	une_node *expression = une_parse_expression(error, ps);
	if (!expression) {
		une_node_free(assignee, false);
		LOGPARSE_END(NULL);
	}
	
	/* Resolve final node kind. */
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
	
	LOGPARSE_END(assignment_or_expression_statement);
}

une_parser__(une_parse_assignee)
{
	LOGPARSE_BEGIN();
	
	bool global = false;
	if (now(&ps->in).kind == UNE_TK_GLOBAL) {
		pull(&ps->in);
		global = true;
	}
	
	une_node *base = une_parse_seek_or_this(error, ps, global);
	if (!base)
		LOGPARSE_END(NULL);
	
	while (true) {
		une_node *accessor;
		if (now(&ps->in).kind == UNE_TK_LSQB) {
			accessor = une_parse_index(error, ps);
			if (accessor)
				accessor->kind = UNE_NK_IDX_SEEK;
		} else if (now(&ps->in).kind == UNE_TK_DOT) {
			accessor = une_parse_member(error, ps);
			if (accessor)
				accessor->kind = UNE_NK_MEMBER_SEEK;
		} else {
			break;
		}
		if (!accessor) {
			une_node_free(base, false);
			LOGPARSE_END(NULL);
		}
		accessor->pos = une_position_set_start(accessor->pos, base->pos);
		accessor->content.branch.a = base;
		base = accessor;
	}
	
	LOGPARSE_END(base);
}

une_parser__(une_parse_index)
{
	LOGPARSE_BEGIN();
	
	/* '['. */
	assert(now(&ps->in).kind == UNE_TK_LSQB);
	pull(&ps->in);
	
	/* Begin? */
	une_node *begin = NULL;
	if (now(&ps->in).kind == UNE_TK_DOTDOT) {
		begin = une_parse_imaginary(error, ps, UNE_NK_INT);
		begin->content.value._int = 0;
	} else {
		begin = une_parse_expression(error, ps);
	}
	if (!begin)
		LOGPARSE_END(NULL);
	
	/* Range? */
	une_node *end = NULL;
	if (now(&ps->in).kind == UNE_TK_DOTDOT) {
		pull(&ps->in); /* '..'. */
		/* End? */
		if (now(&ps->in).kind == UNE_TK_RSQB)
			end = une_parse_imaginary(error, ps, UNE_NK_VOID);
		else
			end = une_parse_expression(error, ps);
		if (!end) {
			une_node_free(begin, false);
			LOGPARSE_END(NULL);
		}
	}
	
	/* ']'. */
	if (now(&ps->in).kind != UNE_TK_RSQB) {
		*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
		une_node_free(begin, false);
		une_node_free(end, false);
		LOGPARSE_END(NULL);
	}
	pull(&ps->in);
	
	une_node *index = une_node_create(UNE_NK_none__); /* The caller is required to provide the node kind. */
	index->pos.end = peek(&ps->in, -1).pos.end; /* The caller is required to provide the start position. */
	index->content.branch.b = begin; /* The caller is required to provide branch A. */
	index->content.branch.c = end;
	LOGPARSE_END(index);
}

une_parser__(une_parse_call)
{
	LOGPARSE_BEGIN();
	
	/* Arguments. */
	une_node *arguments = une_parse_sequence(error, ps, UNE_NK_LIST, UNE_TK_LPAR, UNE_TK_SEP, UNE_TK_RPAR, &une_parse_expression);
	if (!arguments)
		LOGPARSE_END(NULL);
	
	une_node *call = une_node_create(UNE_NK_none__); /* The caller is required to provide the node kind. */
	call->pos.end = arguments->pos.end; /* The caller is required to provide the start position. */
	call->content.branch.b = arguments; /* The caller is required to provide branch A. */
	LOGPARSE_END(call);
}

une_parser__(une_parse_member)
{
	LOGPARSE_BEGIN();
	
	/* '.'. */
	assert(now(&ps->in).kind == UNE_TK_DOT);
	pull(&ps->in);
	
	/* Identifier. */
	une_node *name = une_parse_name(error, ps);
	if (!name)
		LOGPARSE_END(NULL);
	
	une_node *member = une_node_create(UNE_NK_none__); /* The caller is required to provide the node kind. */
	member->pos.end = name->pos.end; /* The caller is required to provide the start position. */
	member->content.branch.b = name; /* The caller is required to provide branch A. */
	LOGPARSE_END(member);
}

une_parser__(une_parse_object_association)
{
	LOGPARSE_BEGIN();
	
	/* Name. */
	une_node *name = une_parse_name(error, ps);
	if (!name)
		LOGPARSE_END(NULL);
	
	/* ':'. */
	if (now(&ps->in).kind != UNE_TK_COLON) {
		*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
		une_node_free(name, false);
		LOGPARSE_END(NULL);
	}
	pull(&ps->in);
	
	/* Expression. */
	une_node *expression = une_parse_expression(error, ps);
	if (!expression) {
		*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
		une_node_free(name, false);
		LOGPARSE_END(NULL);
	}
	
	une_node *object_association = une_node_create(UNE_NK_OBJECT_ASSOCIATION);
	object_association->pos = une_position_between(name->pos, expression->pos);
	object_association->content.branch.a = name;
	object_association->content.branch.b = expression;
	LOGPARSE_END(object_association);
}

une_parser__(une_parse_object)
{
	LOGPARSE_BEGIN();
	
	LOGPARSE_END(une_parse_sequence(error, ps, UNE_NK_OBJECT, UNE_TK_LBRC, UNE_TK_SEP, UNE_TK_RBRC, &une_parse_object_association));
}

une_parser__(une_parse_unary_operation, une_node_kind node_t, une_node *(*parse)(une_error*, une_parser_state*)
)
{
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
	une_token_kind range_begin,
	une_node_kind range_begin_nt,
	une_token_kind range_end,
	une_node *(*parse_left)(une_error*, une_parser_state*),
	une_node *(*parse_right)(une_error*, une_parser_state*)
)
{
	une_node *left = (*parse_left)(error, ps);
	if (left == NULL)
		return NULL;

	while (now(&ps->in).kind >= range_begin && now(&ps->in).kind <= range_end)
	{
		une_node_kind kind = range_begin_nt+(une_node_kind)(now(&ps->in).kind-range_begin);

		pull(&ps->in);

		une_node *right = (*parse_right)(error, ps);
		if (right == NULL) {
			une_node_free(left, false);
			return NULL;
		}

		une_node *new_left = une_node_create(kind);
		new_left->pos = une_position_between(left->pos, right->pos);
		new_left->content.branch.a = left;
		new_left->content.branch.b = right;
		left = new_left;
	}
	
	return left;
}

une_parser__(une_parse_sequence,
	une_node_kind node_kind,
	une_token_kind begin, /* If omitted, the sequence starts at the next non-whitespace token. */
	une_token_kind delimiter, /* If omitted, we only allow NEW between elements. */
	une_token_kind end, /* If omitted, the sequence stops immediately after one element has been parsed. */
	une_node *(*parser)(une_error*, une_parser_state*)
)
{
	/* Begin the sequence. */
	une_position pos_first = now(&ps->in).pos;
	if (begin != UNE_TK_none__) {
		if (now(&ps->in).kind != begin) {
			*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
			return NULL;
		}
		pull(&ps->in);
	}
	
	size_t sequence_size = UNE_SIZE_SEQUENCE;
	une_node **sequence = malloc(sequence_size*sizeof(*sequence));
	verify(sequence);
	size_t sequence_index = 1;

	while (true) {
		if (sequence_index >= sequence_size) {
			sequence_size *= 2;
			sequence = realloc(sequence, sequence_size*sizeof(*sequence));
			verify(sequence);
		}
		
		/* Skip whitespace and, if provided, the delimeter. */
		while (
			now(&ps->in).kind == UNE_TK_NEW ||
			(delimiter != UNE_TK_none__ && now(&ps->in).kind == delimiter)
		)
			pull(&ps->in);

		/* If an end was provided and the current token matches it, stop the sequence. */
		if (end != UNE_TK_none__ && now(&ps->in).kind == end)
			break;

		/* Stop the sequence if we prematurely hit EOF. This can happen if a sequence is
		opened at the end of the file without being closed. */
		if (now(&ps->in).kind == UNE_TK_EOF) {
			for (size_t i=1; i<sequence_index; i++)
				une_node_free(sequence[i], false);
			free(sequence);
			*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
			return NULL;
		}

		/* Parse an element. */
		sequence[sequence_index] = (*parser)(error, ps);
		if (sequence[sequence_index] == NULL) {
			for (size_t i=1; i<sequence_index; i++)
				une_node_free(sequence[i], false);
			free(sequence);
			return NULL;
		}
		sequence_index++;

		/* If no end was provided, stop the sequence here. It's important that we leave subsequent whitespace
		intact, because those tokens might be needed as delimiters in encompassing sequences. */
		if (end == UNE_TK_none__)
			break;
		
		/* Skip whitespace after the element. */
		if (delimiter != UNE_TK_NEW)
			une_parser_skip_whitespace(ps);
		
		/* If neither end nor a delimiter follows, we hit an unexpected token. */
		if (now(&ps->in).kind != end && now(&ps->in).kind != delimiter) {
			for (size_t i=1; i<sequence_index; i++)
				une_node_free(sequence[i], false);
			free(sequence);
			*error = UNE_ERROR_SET(UNE_EK_SYNTAX, now(&ps->in).pos);
			return NULL;
		}
	}

	/* Create the container node. */
	une_node *counter = une_node_create(UNE_NK_SIZE);
	counter->content.value._int = (une_int)sequence_index-1;
	sequence[0] = counter;
	une_node *node = une_node_create(node_kind);
	node->pos = une_position_between(pos_first, now(&ps->in).pos);
	node->content.value._vpp = (void**)sequence;
	
	/* Conclude the sequence. We don't skip EOF because it may
	still be needed by other functions up the call chain. */
	if (end != UNE_TK_none__ && end != UNE_TK_EOF)
		pull(&ps->in);
	
	return node;
}

une_parser__(une_parse_imaginary,
	une_node_kind node_kind
)
{
	une_node *phony = une_node_create(node_kind);
	phony->pos = une_position_set_start(phony->pos, now(&ps->in).pos);
	phony->pos.end = phony->pos.start;
	return phony;
}

ptrdiff_t une_parser_checkpoint(une_parser_state *ps)
{
	return ps->in.index;
}

void une_parser_return_to_checkpoint(une_error *error, une_parser_state *ps, ptrdiff_t checkpoint)
{
	/* Discard errors. */
	error->kind = UNE_EK_none__;

	/* Step back. */
	ps->in.index = checkpoint;
}

void une_parser_skip_whitespace(une_parser_state *ps)
{
	while (now(&ps->in).kind == UNE_TK_NEW)
		pull(&ps->in);
}
