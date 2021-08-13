/*
node.h - Une
Modified 2021-08-13
*/

#ifndef UNE_NODE_H
#define UNE_NODE_H

/* Header-specific includes. */
#include "../primitive.h"

/*
Type of une_node.
*/
typedef enum _une_node_type {
  __UNE_NT_none__,
  UNE_NT_ID,
  UNE_NT_SIZE,
  #define UNE_R_BGN_LUT_NODES UNE_NT_INT /* (!) Order-sensitive. */
  UNE_NT_INT,
  UNE_NT_FLT,
  UNE_NT_STR,
  UNE_NT_LIST,
  UNE_NT_FUNCTION,
  UNE_NT_BUILTIN,
  UNE_NT_STMTS,
  UNE_NT_COP,
  UNE_NT_NOT,
  #define UNE_R_BGN_AND_OR_NODES UNE_NT_AND
  UNE_NT_AND,
  UNE_NT_OR,
  #define UNE_R_END_AND_OR_NODES UNE_NT_OR
  #define UNE_R_BGN_CONDITION_NODES UNE_NT_EQU
  UNE_NT_EQU,
  UNE_NT_NEQ,
  UNE_NT_GTR,
  UNE_NT_GEQ,
  UNE_NT_LSS,
  UNE_NT_LEQ,
  #define UNE_R_END_CONDITION_NODES UNE_NT_LEQ
  #define UNE_R_BGN_ADD_SUB_NODES UNE_NT_ADD
  UNE_NT_ADD,
  UNE_NT_SUB,
  #define UNE_R_END_ADD_SUB_NODES UNE_NT_SUB
  #define UNE_R_BEGIN_TERM_NODES UNE_NT_MUL
  UNE_NT_MUL,
  UNE_NT_DIV,
  UNE_NT_FDIV,
  UNE_NT_MOD,
  #define UNE_R_END_TERM_NODES UNE_NT_MOD
  #define UNE_R_BGN_POWER_NODES UNE_NT_POW
  UNE_NT_POW,
  #define UNE_R_END_POWER_NODES UNE_NT_POW
  UNE_NT_NEG,
  UNE_NT_SET,
  UNE_NT_SET_IDX,
  UNE_NT_GET,
  UNE_NT_GET_IDX,
  UNE_NT_CALL,
  UNE_NT_FOR,
  UNE_NT_WHILE,
  UNE_NT_IF,
  UNE_NT_CONTINUE,
  UNE_NT_BREAK,
  UNE_NT_RETURN,
  #define UNE_R_END_LUT_NODES UNE_NT_RETURN /* (!) Order-sensitive. */
  __UNE_NT_max__,
} une_node_type;

/*
A program node, holding either more nodes or data.
*/
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

/*
*** Interface.
*/

/*
Condition to check whether une_node_type is valid.
*/
#define UNE_NODE_TYPE_IS_VALID(type)\
  (type > __UNE_NT_none__ && type < __UNE_NT_max__)

/*
Condition to check whether une_node_type is in interpreter lookup table.
*/
#define UNE_NODE_TYPE_IS_IN_LUT(type)\
  (type >= UNE_R_BGN_LUT_NODES && type <= UNE_R_END_LUT_NODES)

/*
Unpack a une_node list into its name and size.
*/
#define UNE_UNPACK_NODE_LIST(listnode, listname, listsize)\
  une_node **listname = (une_node**)listnode->content.value._vpp;\
  assert(listname != NULL);\
  size_t listsize = listname[0]->content.value._int

/*
Iterate over every item in a une_node list.
*/
#define UNE_FOR_NODE_LIST_ITEM(var, size)\
  for (size_t var=1; var<=size; var++)

/*
Iterate over every index in a une_node list.
*/
#define UNE_FOR_NODE_LIST_INDEX(var, size)\
  for (size_t var=0; var<=size; var++)

une_node *une_node_create(une_node_type type);
une_node *une_node_copy(une_node *src);
void une_node_free(une_node *node, bool free_wcs);

une_node **une_node_list_create(size_t size);

#ifdef UNE_DEBUG
__une_static const wchar_t *une_node_type_to_wcs(une_node_type type);
wchar_t *une_node_to_wcs(une_node *node);
#endif /* UNE_DEBUG */

#endif /* !UNE_NODE_H */
