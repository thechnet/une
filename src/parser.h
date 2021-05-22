/*
parser.h - Une
Updated 2021-05-22
*/

#ifndef UNE_PARSER_H
#define UNE_PARSER_H

#include "primitive.h"
#include "types/error.h"
#include "types/token.h"
#include "types/node.h"
#include "types/instance.h"
#include <string.h>
#include "tools.h"

// Public Parser Interface.
une_node *une_parse(une_instance *inst);

// Token Stream (FIXME: Remove these? - Also see the special case in une_parse_for)
static une_token une_p_peek(une_instance *inst);
static void une_p_consume(une_instance *inst);
static size_t une_p_index_get(une_instance *inst);
static void une_p_index_set(une_instance *inst, size_t index);

// Parsers.
static une_node *une_parse_stmt      (une_instance *inst);
static une_node *une_parse_if        (une_instance *inst);
static une_node *une_parse_for       (une_instance *inst);
static une_node *une_parse_while     (une_instance *inst);
static une_node *une_parse_def       (une_instance *inst);
static une_node *une_parse_return    (une_instance *inst);
static une_node *une_parse_expression(une_instance *inst);
static une_node *une_parse_and_or    (une_instance *inst);
static une_node *une_parse_condition (une_instance *inst);
static une_node *une_parse_add_sub   (une_instance *inst);
static une_node *une_parse_term      (une_instance *inst);
static une_node *une_parse_power     (une_instance *inst);
static une_node *une_parse_index     (une_instance *inst);
static une_node *une_parse_call      (une_instance *inst);
static une_node *une_parse_atom      (une_instance *inst);
static une_node *une_parse_id        (une_instance *inst);

// Helpers.
static une_node *une_parse_sequence(
  une_instance *inst,
  une_node_type node_type,
  une_token_type tt_begin, une_token_type tt_end_of_item, une_token_type tt_end,
  une_node* (*parser)(une_instance*)
);
static une_node *une_parse_unary_operation(
  une_instance *inst,
  une_node_type node_t,
  une_node* (*parse)(une_instance*)
);
static une_node *une_parse_binary_operation(
  une_instance *inst,
  une_token_type range_begin_tt,
  une_node_type range_begin_nt,
  une_token_type range_end_tt,
  une_node_type range_end_nt,
  une_node* (*parse_left)(une_instance*),
  une_node* (*parse_right)(une_instance*)
);

#endif /* !UNE_PARSER_H */
