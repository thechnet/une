/*
node.h - Une
Updated 2021-05-01
*/

#ifndef UNE_NODE_H
#define UNE_NODE_H

#include "../primitive.h"
#include "../tools.h"

#pragma region une_node_type
typedef enum _une_node_type {
  __UNE_NT_none__,
  
  UNE_NT_INT,
  UNE_NT_FLT,
  UNE_NT_STR,
  UNE_NT_ID,
  UNE_NT_LIST,
  UNE_NT_SIZE,
  UNE_NT_STMTS,

  UNE_NT_COP,

  UNE_NT_NOT,
  // Begin And/Or Nodes
  UNE_NT_AND,
  UNE_NT_OR,
  // Begin And/Or Nodes

  // Begin Condition Nodes
  UNE_NT_EQU,
  UNE_NT_NEQ,
  UNE_NT_GTR,
  UNE_NT_GEQ,
  UNE_NT_LSS,
  UNE_NT_LEQ,
  // End Condition Nodes

  // Begin Add/Sub Nodes
  UNE_NT_ADD,
  UNE_NT_SUB,
  // End Add/Sub Nodes

  // Begin Term Nodes
  UNE_NT_MUL,
  UNE_NT_DIV,
  UNE_NT_FDIV,
  UNE_NT_MOD,
  // End Term Nodes

  UNE_NT_POW,
  UNE_NT_NEG,

  UNE_NT_SET,
  UNE_NT_SET_IDX,
  UNE_NT_GET,
  UNE_NT_GET_IDX,
  UNE_NT_DEF,
  UNE_NT_CALL,
  UNE_NT_FOR,
  UNE_NT_WHILE,
  UNE_NT_IF,
  UNE_NT_CONTINUE,
  UNE_NT_BREAK,
  UNE_NT_RETURN,

  __UNE_NT_max__
} une_node_type;
#pragma endregion une_node_type

#pragma region une_node
typedef struct _une_node {
  une_node_type type;
  une_position pos;
  union _content {
    une_value value;
    struct _branch {
        struct _une_node *a;
        struct _une_node *b;
        struct _une_node *c;
        struct _une_node *d;
    } branch;
  } content;
} une_node;
#pragma endregion une_node

const wchar_t *une_node_type_to_wcs(une_node_type type);
wchar_t *une_node_to_wcs(une_node *node);
void une_node_free(une_node *node, bool free_wcs);
une_node *une_node_create(une_node_type type);
une_node *une_node_copy(une_node *src);

#define UNE_UNPACK_NODE_LIST(listnode, listname, listsize)\
  une_node **listname = (une_node**)listnode->content.value._vpp;\
  size_t listsize = listname[0]->content.value._int;

#define UNE_FOR_NODE_LIST_ITEM(var, size)\
  for (size_t var=1; var<=size; var++)

#endif /* !UNE_NODE_H */
