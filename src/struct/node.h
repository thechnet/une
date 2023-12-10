/*
node.h - Une
Modified 2023-12-10
*/

#ifndef UNE_NODE_H
#define UNE_NODE_H

/* Header-specific includes. */
#include "../common.h"

/*
Kind of une_node.
*/
typedef enum une_node_kind_ {
	UNE_NK_none__,
	UNE_NK_NAME,
	UNE_NK_SIZE,
	UNE_NK_ID,
	#define UNE_R_BGN_LUT_NODES UNE_NK_VOID /* (!) Order-sensitive. */
	UNE_NK_VOID,
	UNE_NK_INT,
	UNE_NK_FLT,
	UNE_NK_STR,
	UNE_NK_LIST,
	UNE_NK_OBJECT,
	UNE_NK_FUNCTION,
	UNE_NK_NATIVE,
	UNE_NK_STMTS,
	UNE_NK_COP,
	UNE_NK_NOT,
	#define UNE_R_BGN_AND_OR_NODES UNE_NK_AND
	UNE_NK_AND,
	UNE_NK_OR,
	UNE_NK_NULLISH,
	#define UNE_R_END_AND_OR_NODES UNE_NK_NULLISH
	#define UNE_R_BGN_CONDITION_NODES UNE_NK_EQU
	UNE_NK_EQU,
	UNE_NK_NEQ,
	UNE_NK_GEQ,
	UNE_NK_GTR,
	UNE_NK_LEQ,
	UNE_NK_LSS,
	#define UNE_R_END_CONDITION_NODES UNE_NK_LSS
	#define UNE_R_BGN_ADD_SUB_NODES UNE_NK_ADD
	UNE_NK_ADD,
	UNE_NK_SUB,
	#define UNE_R_END_ADD_SUB_NODES UNE_NK_SUB
	#define UNE_R_BEGIN_TERM_NODES UNE_NK_MUL
	UNE_NK_MUL,
	UNE_NK_FDIV,
	UNE_NK_DIV,
	UNE_NK_MOD,
	#define UNE_R_END_TERM_NODES UNE_NK_MOD
	UNE_NK_POW,
	UNE_NK_NEG,
	UNE_NK_SEEK,
	UNE_NK_IDX_SEEK,
	UNE_NK_MEMBER_SEEK,
	#define UNE_R_BGN_ASSIGNMENT_NODES UNE_NK_ASSIGN
	UNE_NK_ASSIGN,
	UNE_NK_ASSIGNADD,
	UNE_NK_ASSIGNSUB,
	UNE_NK_ASSIGNPOW,
	UNE_NK_ASSIGNMUL,
	UNE_NK_ASSIGNFDIV,
	UNE_NK_ASSIGNDIV,
	UNE_NK_ASSIGNMOD,
	#define UNE_R_END_ASSIGNMENT_NODES UNE_NK_ASSIGNMOD
	UNE_NK_CALL,
	UNE_NK_FOR_RANGE,
	UNE_NK_FOR_ELEMENT,
	UNE_NK_WHILE,
	UNE_NK_IF,
	UNE_NK_ASSERT,
	UNE_NK_CONTINUE,
	UNE_NK_BREAK,
	UNE_NK_RETURN,
	UNE_NK_EXIT,
	UNE_NK_ANY,
	UNE_NK_ALL,
	UNE_NK_COVER,
	UNE_NK_CONCATENATE,
	UNE_NK_THIS,
	#define UNE_R_END_LUT_NODES UNE_NK_THIS /* (!) Order-sensitive. */
	UNE_NK_OBJECT_ASSOCIATION,
	UNE_NK_max__,
} une_node_kind;

/*
A program node, holding either more nodes or data.
*/
typedef struct une_node_ {
	une_node_kind kind;
	une_position pos;
	union content_ {
		une_value value;
		struct branch_ {
				struct une_node_ *a;
				struct une_node_ *b;
				struct une_node_ *c;
				struct une_node_ *d;
		} branch;
	} content;
} une_node;

/*
*** Interface.
*/

/*
Condition to check whether une_node_kind is valid.
*/
#define UNE_NODE_KIND_IS_VALID(kind)\
	(kind > UNE_NK_none__ && kind < UNE_NK_max__)

/*
Condition to check whether une_node_kind is in interpreter lookup table.
*/
#define UNE_NODE_KIND_IS_IN_LUT(kind)\
	(kind >= UNE_R_BGN_LUT_NODES && kind <= UNE_R_END_LUT_NODES)

/*
Unpack a une_node list into its name and size.
*/
#define UNE_UNPACK_NODE_LIST(listnode, listname, listsize)\
	une_node **listname = (une_node**)listnode->content.value._vpp;\
	assert(listname != NULL);\
	size_t listsize = (size_t)listname[0]->content.value._int

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

une_node *une_node_create(une_node_kind kind);
une_node *une_node_copy(une_node *src);
void une_node_free(une_node *node, bool free_wcs);

une_node **une_node_list_create(size_t size);

une_node *une_node_unwrap_any_or_all(une_node *node, une_node_kind *wrapped_as);

#ifdef UNE_DEBUG
une_static__ const wchar_t *une_node_kind_to_wcs(une_node_kind kind);
wchar_t *une_node_to_wcs(une_node *node);
#endif /* UNE_DEBUG */

#endif /* !UNE_NODE_H */
