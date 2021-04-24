/*
parser.h - Une
Updated 2021-04-24
*/

#ifndef UNE_PARSER_H
#define UNE_PARSER_H

#include "primitive.h"
#include "types/error.h"
#include "types/token.h"
#include "types/node.h"
#include <string.h>

une_node *une_parse_block(une_token *tokens, size_t *token_index, une_error *error);
une_node **une_parse_sequence(
  une_token *tokens, size_t *token_index, une_error *error,
  une_node *(*parser)(une_token *tokens, size_t *token_index, une_error *error),
  une_token_type tt_end_of_item,
  une_token_type tt_end_of_sequence
);
une_node *une_parse_stmt(
  une_token *tokens,
  size_t *token_index,
  une_error *error
  /*bool is_loop_body*/
);
une_node *une_parse_if(une_token *tokens, size_t *token_index, une_error *error, une_token_type start_token);
une_node *une_parse_for(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_while(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_def(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_return(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_conditional_operation(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_conditions(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_condition(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_expression(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_term(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_power(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_index(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_atom(une_token *tokens, size_t *token_index, une_error *error);
une_node *une_parse_id(une_token *tokens, size_t *token_index, une_error *error);

#endif /* !UNE_PARSER_H */
