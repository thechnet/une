/*
parser_state.h - Une
Modified 2021-06-13
*/

#ifndef UNE_PARSER_STATE_H
#define UNE_PARSER_STATE_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../stream.h"
#include "token.h"
#include "node.h"

/*
Holds the state of the parser.
*/
typedef struct _une_parser_state {
  bool inside_loop;
  une_istream in;
  une_node (*pull)(une_istream*);
  une_node (*peek)(une_istream*, ptrdiff_t);
  une_node (*now)(une_istream*);
} une_parser_state;

/*
*** Interface.
*/

une_parser_state une_parser_state_create(void);

#endif /* UNE_PARSER_STATE_H */
