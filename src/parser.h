/*
parser.h - Une
Updated 2021-06-04
*/

#ifndef UNE_PARSER_H
#define UNE_PARSER_H

#include "primitive.h"
#include "types/error.h"
#include "types/token.h"
#include "types/node.h"
#include "types/parser_state.h"
#include <string.h>
#include "tools.h"

// Public Parser Interface.
une_node *une_parse(une_error *error, une_parser_state *ps);

// Token Stream (FIXME: Remove these? - Also see the special case in une_parse_for)
static une_token une_p_peek(une_parser_state *ps);
static void une_p_consume(une_parser_state *ps);
static size_t une_p_index_get(une_parser_state *ps);
static void une_p_index_set(une_parser_state *ps, size_t index);

// Parsers.
static une_node *une_parse_stmt      (une_error *error, une_parser_state *ps);
static une_node *une_parse_if        (une_error *error, une_parser_state *ps);
static une_node *une_parse_for       (une_error *error, une_parser_state *ps);
static une_node *une_parse_while     (une_error *error, une_parser_state *ps);
static une_node *une_parse_def       (une_error *error, une_parser_state *ps);
static une_node *une_parse_return    (une_error *error, une_parser_state *ps);
static une_node *une_parse_expression(une_error *error, une_parser_state *ps);
static une_node *une_parse_and_or    (une_error *error, une_parser_state *ps);
static une_node *une_parse_condition (une_error *error, une_parser_state *ps);
static une_node *une_parse_add_sub   (une_error *error, une_parser_state *ps);
static une_node *une_parse_term      (une_error *error, une_parser_state *ps);
static une_node *une_parse_power     (une_error *error, une_parser_state *ps);
static une_node *une_parse_index     (une_error *error, une_parser_state *ps);
static une_node *une_parse_call      (une_error *error, une_parser_state *ps);
static une_node *une_parse_atom      (une_error *error, une_parser_state *ps);
static une_node *une_parse_id        (une_error *error, une_parser_state *ps);

// Helpers.
static une_node *une_parse_sequence(
  une_error *error,
  une_parser_state *ps,
  une_node_type node_type,
  une_token_type tt_begin, une_token_type tt_end_of_item, une_token_type tt_end,
  une_node* (*parser)(une_error*, une_parser_state*)
);
static une_node *une_parse_unary_operation(
  une_error *error,
  une_parser_state *ps,
  une_node_type node_t,
  une_node* (*parse)(une_error*, une_parser_state*)
);
static une_node *une_parse_binary_operation(
  une_error *error,
  une_parser_state *ps,
  une_token_type range_begin_tt,
  une_node_type range_begin_nt,
  une_token_type range_end_tt,
  une_node_type range_end_nt,
  une_node* (*parse_left)(une_error*, une_parser_state*),
  une_node* (*parse_right)(une_error*, une_parser_state*)
);

#endif /* !UNE_PARSER_H */
