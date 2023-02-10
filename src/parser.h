/*
parser.h - Une
Modified 2023-02-10
*/

#ifndef UNE_PARSER_H
#define UNE_PARSER_H

/* Header-specific includes. */
#include "primitive.h"
#include "types/error.h"
#include "types/token.h"
#include "types/node.h"
#include "types/parser_state.h"

/*
*** Interface.
*/

/*
Parser function template.
*/
#define une_parser__(id__, ...)\
  une_static__ une_node *(id__)(une_error *error, une_parser_state *ps, ##__VA_ARGS__)

une_node *une_parse(une_error *error, une_parser_state *ps, une_token *tokens);

une_parser__(une_parse_stmt);
une_parser__(une_parse_id);
une_parser__(une_parse_block);
une_parser__(une_parse_expression);
une_parser__(une_parse_and_or);
une_parser__(une_parse_condition);
une_parser__(une_parse_cover);
une_parser__(une_parse_add_sub);
une_parser__(une_parse_term);
une_parser__(une_parse_negation);
une_parser__(une_parse_power);
une_parser__(une_parse_index_or_call);
une_parser__(une_parse_atom);
une_parser__(une_parse_void);
une_parser__(une_parse_int);
une_parser__(une_parse_flt);
une_parser__(une_parse_str);
une_parser__(une_parse_true);
une_parser__(une_parse_false);
une_parser__(une_parse_get);
une_parser__(une_parse_seek, bool global);
une_parser__(une_parse_builtin);
une_parser__(une_parse_list);
une_parser__(une_parse_function);
une_parser__(une_parse_for);
une_parser__(une_parse_for_range);
une_parser__(une_parse_for_element);
une_parser__(une_parse_while);
une_parser__(une_parse_if);
une_parser__(une_parse_continue);
une_parser__(une_parse_break);
une_parser__(une_parse_return);
une_parser__(une_parse_exit);
une_parser__(une_parse_assignment_or_expr_stmt);
une_parser__(une_parse_assignee);

une_parser__(une_parse_unary_operation,
  une_node_type node_t,
  une_node *(*parse)(une_error*, une_parser_state*)
);

une_parser__(une_parse_binary_operation,
  une_token_type range_begin_tt,
  une_node_type range_begin_nt,
  une_token_type range_end_tt,
  une_node *(*parse_left)(une_error*, une_parser_state*),
  une_node *(*parse_right)(une_error*, une_parser_state*)
);

une_parser__(une_parse_sequence,
  une_node_type node_type,
  une_token_type tt_begin,
  une_token_type tt_end_of_item,
  une_token_type tt_end,
  une_node *(*parser)(une_error*, une_parser_state*)
);

#endif /* !UNE_PARSER_H */
