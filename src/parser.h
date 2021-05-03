/*
parser.h - Une
Updated 2021-05-03
*/

#ifndef UNE_PARSER_H
#define UNE_PARSER_H

#include "primitive.h"
#include "types/error.h"
#include "types/token.h"
#include "types/node.h"
#include <string.h>
#include "tools.h"

une_node *une_parse(une_token *tokens, size_t *token_index, une_error *error);

static une_node *une_parse_stmt(
  une_token *tokens,
  size_t *token_index,
  une_error *error
  /*bool is_loop_body*/
);

static une_node *une_parse_sequence(
  une_token *tokens, size_t *token_index, une_error *error,
  une_node_type node_type,
  une_token_type tt_begin, une_token_type tt_end_of_item, une_token_type tt_end,
  une_node* (*parser)(une_token*, size_t*, une_error*)
);

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
);

static une_node *une_parse_unary_operation(
  une_token *tokens,
  size_t *token_index,
  une_error *error,
  une_node_type node_t,
  une_node* (*parse)(une_token*, size_t*, une_error*)
);

static une_node *une_parse_if(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_for(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_while(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_def(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_return(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_expression(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_and_or(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_condition(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_add_sub(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_term(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_power(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_index(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_call(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_atom(une_token *tokens, size_t *token_index, une_error *error);
static une_node *une_parse_id(une_token *tokens, size_t *token_index, une_error *error);

#endif /* !UNE_PARSER_H */
