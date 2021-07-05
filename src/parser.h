/*
parser.h - Une
Modified 2021-07-05
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
#define __une_parser(__id, ...)\
  /*__une_static*/ une_node *(__id)(une_error *error, une_parser_state *ps, ##__VA_ARGS__)

une_node *une_parse(une_error *error, une_parser_state *ps, une_token *tokens);

__une_parser(une_parse_stmt);
__une_parser(une_parse_id);
__une_parser(une_parse_block);
__une_parser(une_parse_expression);
__une_parser(une_parse_and_or);
__une_parser(une_parse_condition);
__une_parser(une_parse_add_sub);
__une_parser(une_parse_term);
__une_parser(une_parse_power);
__une_parser(une_parse_index);
__une_parser(une_parse_atom);
__une_parser(une_parse_def);
__une_parser(une_parse_for);
__une_parser(une_parse_while);
__une_parser(une_parse_if);
__une_parser(une_parse_continue);
__une_parser(une_parse_break);
__une_parser(une_parse_return);
__une_parser(une_parse_set_expstmt);

__une_parser(une_parse_unary_operation,
  une_node_type node_t,
  une_node* (*parse)(une_error*, une_parser_state*)
);

__une_parser(une_parse_binary_operation,
  une_token_type range_begin_tt,
  une_node_type range_begin_nt,
  une_token_type range_end_tt,
  une_node_type range_end_nt,
  une_node* (*parse_left)(une_error*, une_parser_state*),
  une_node* (*parse_right)(une_error*, une_parser_state*)
);

__une_parser(une_parse_sequence,
  une_node_type node_type,
  une_token_type tt_begin,
  une_token_type tt_end_of_item,
  une_token_type tt_end,
  une_node* (*parser)(une_error*, une_parser_state*)
);

#endif /* !UNE_PARSER_H */
