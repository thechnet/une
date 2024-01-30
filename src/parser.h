/*
parser.h - Une
Modified 2024-01-31
*/

#ifndef UNE_PARSER_H
#define UNE_PARSER_H

/* Header-specific includes. */
#include "common.h"
#include "struct/error.h"
#include "struct/token.h"
#include "struct/node.h"
#include "struct/parser_state.h"

/*
*** Interface.
*/

/*
Parser function template.
*/
#define une_parser__(name__, ...)\
	une_static__ une_node *(name__)(une_error *error, une_parser_state *ps, ##__VA_ARGS__)

une_node *une_parse(une_error *error, une_parser_state *ps, une_token *tokens);

une_parser__(une_parse_stmt);
une_parser__(une_parse_name);
une_parser__(une_parse_block);
une_parser__(une_parse_expression);
une_parser__(une_parse_conditional);
une_parser__(une_parse_and_or);
une_parser__(une_parse_condition);
une_parser__(une_parse_any_all);
une_parser__(une_parse_cover);
une_parser__(une_parse_add_sub);
une_parser__(une_parse_term);
une_parser__(une_parse_negation);
une_parser__(une_parse_minus);
une_parser__(une_parse_power);
une_parser__(une_parse_accessor);
une_parser__(une_parse_atom);
une_parser__(une_parse_void);
une_parser__(une_parse_int);
une_parser__(une_parse_flt);
une_parser__(une_parse_str);
une_parser__(une_parse_true);
une_parser__(une_parse_false);
une_parser__(une_parse_this);
une_parser__(une_parse_seek, bool global);
une_parser__(une_parse_native);
une_parser__(une_parse_list);
une_parser__(une_parse_signature);
une_parser__(une_parse_function, une_node *parameters);
une_parser__(une_parse_for);
une_parser__(une_parse_for_range);
une_parser__(une_parse_for_element);
une_parser__(une_parse_while);
une_parser__(une_parse_if);
une_parser__(une_parse_assert);
une_parser__(une_parse_continue);
une_parser__(une_parse_break);
une_parser__(une_parse_return);
une_parser__(une_parse_exit);
une_parser__(une_parse_assignment_or_expr_stmt);
une_parser__(une_parse_assignee);
une_parser__(une_parse_index);
une_parser__(une_parse_call);
une_parser__(une_parse_member);
une_parser__(une_parse_object_association);
une_parser__(une_parse_object);

une_parser__(une_parse_unary_operation,
	une_node_kind node_t,
	une_node *(*parse)(une_error*, une_parser_state*)
);

une_parser__(une_parse_binary_operation,
	une_token_kind range_begin,
	une_node_kind range_begin_nt,
	une_token_kind range_end,
	une_node *(*parse_left)(une_error*, une_parser_state*),
	une_node *(*parse_right)(une_error*, une_parser_state*)
);

une_parser__(une_parse_sequence,
	une_node_kind node_kind,
	une_token_kind begin,
	une_token_kind end_of_item,
	une_token_kind end,
	une_node *(*parser)(une_error*, une_parser_state*)
);

une_parser__(une_parse_as_sequence,
	une_node_kind node_kind,
	une_node *(*parser)(une_error*, une_parser_state*)
);

une_parser__(une_parse_phony,
	une_node_kind node_kind
);

ptrdiff_t une_parser_checkpoint(une_parser_state *ps);
void une_parser_return_to_checkpoint(une_error *error, une_parser_state *ps, ptrdiff_t checkpoint);

void une_parser_skip_whitespace(une_parser_state *ps);

#endif /* !UNE_PARSER_H */
