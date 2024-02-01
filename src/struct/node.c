/*
node.c - Une
Modified 2024-11-09
*/

/* Header-specific includes. */
#include "node.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "../natives.h"

/*
Node name table.
*/
const wchar_t *une_node_table[] = {
	L"NAME",
	L"SIZE",
	L"ID",
	L"VOID",
	L"INT",
	L"FLT",
	L"STR",
	L"LIST",
	L"OBJECT",
	L"FUNCTION",
	L"NATIVE",
	L"STMTS",
	L"COP",
	L"NOT",
	L"AND",
	L"OR",
	L"NULLISH",
	L"EQU",
	L"NEQ",
	L"GEQ",
	L"GTR",
	L"LEQ",
	L"LSS",
	L"ADD",
	L"SUB",
	L"MUL",
	L"FDIV",
	L"DIV",
	L"MOD",
	L"POW",
	L"NEG",
	L"SEEK",
	L"IDX_SEEK",
	L"MEMBER_SEEK",
	L"ASSIGN",
	L"ASSIGNADD",
	L"ASSIGNSUB",
	L"ASSIGNPOW",
	L"ASSIGNMUL",
	L"ASSIGNFDIV",
	L"ASSIGNDIV",
	L"ASSIGNMOD",
	L"CALL",
	L"FOR_RANGE",
	L"FOR_ELEMENT",
	L"WHILE",
	L"IF",
	L"ASSERT",
	L"CONTINUE",
	L"BREAK",
	L"RETURN",
	L"EXIT",
	L"ANY",
	L"ALL",
	L"COVER",
	L"CONCATENATE",
	L"THIS",
	L"OBJECT_ASSOCIATION",
};

/*
Allocate, initialize, and return a une_node struct pointer.
*/
une_node *une_node_create(une_node_kind kind)
{
	/* Allocate new une_node. */
	une_node *node = malloc(sizeof(*node));
	verify(node);
	
	/* Initialize new une_node. */
	*node = (une_node){
		.kind = kind,
		.pos = (une_position){ 0 },
		.content.branch.a = NULL,
		.content.branch.b = NULL,
		.content.branch.c = NULL,
		.content.branch.d = NULL
	};
	
	return node;
}

/*
Duplicate and return a pointer to a une_node struct.
Special with this function is that is also duplicates to string pointers,
which nodes normally just inherit from tokens. This allows function bodies
to persist after a file's tokens have been freed.
*/
une_node *une_node_copy(une_node *src)
{
	/* Ensure source une_node exists. */
	if (src == NULL)
		return NULL;
	
	/* Create destination une_node. */
	une_node *dest = une_node_create(src->kind);
	
	/* Populate destination une_node. */
	dest->pos = src->pos;
	switch (src->kind) {
		
		/* Stack data. */
		case UNE_NK_VOID:
		case UNE_NK_INT:
		case UNE_NK_FLT:
		case UNE_NK_BREAK:
		case UNE_NK_CONTINUE:
		case UNE_NK_SIZE:
		case UNE_NK_ID:
		case UNE_NK_NATIVE:
		case UNE_NK_THIS:
			dest->content.value = src->content.value;
			break;
		
		/* Heap data. */
		case UNE_NK_STR:
		case UNE_NK_NAME:
			dest->content.value._wcs = wcsdup(src->content.value._wcs);
			verify(dest->content.value._wcs);
			break;
		
		/* Node lists. */
		case UNE_NK_LIST:
		case UNE_NK_OBJECT:
		case UNE_NK_STMTS: {
			UNE_UNPACK_NODE_LIST(src, list, size);
			une_node **new_list = une_node_list_create(size);
			UNE_FOR_NODE_LIST_ITEM(i, size)
				new_list[i] = une_node_copy(list[i]);
			dest->content.value._vpp = (void**)new_list;
			break;
		}
		
		/* Nodes. */
		default:
			dest->content.branch.a = une_node_copy(src->content.branch.a);
			if (src->kind == UNE_NK_SEEK)
				dest->content.branch.b = src->content.branch.b;
			else
				dest->content.branch.b = une_node_copy(src->content.branch.b);
			dest->content.branch.c = une_node_copy(src->content.branch.c);
			dest->content.branch.d = une_node_copy(src->content.branch.d);
			break;
	
	}
	
	return dest;
}

/*
Free a une_node struct and all its owned members.
*/
void une_node_free(une_node *node, bool free_wcs)
{
	/* Ensure une_node is valid. */
	if (node == NULL)
		return;
	assert(UNE_NODE_KIND_IS_VALID(node->kind));
	
	/* Free une_node members. */
	switch (node->kind) {
		
		/* Stack data. */
		case UNE_NK_VOID:
		case UNE_NK_INT:
		case UNE_NK_FLT:
		case UNE_NK_NATIVE:
		case UNE_NK_BREAK:
		case UNE_NK_CONTINUE:
		case UNE_NK_SIZE:
		case UNE_NK_ID:
		case UNE_NK_THIS:
			break;
		
		/* Heap data. */
		case UNE_NK_STR:
		case UNE_NK_NAME:
			/* DOC: Memory Management
			We don't normally free the WCS pointers because they are pointing at data stored in the tokens,
			which may still be needed after the parse. This memory is freed alongside the tokens. */
			if (free_wcs)
				free(node->content.value._wcs);
			break;
		
		/* Node lists. */
		case UNE_NK_LIST:
		case UNE_NK_OBJECT:
		case UNE_NK_STMTS: {
			une_node **list = (une_node**)node->content.value._vpp;
			if (list == NULL)
				break;
			size_t list_size = (size_t)list[0]->content.value._int;
			UNE_FOR_NODE_LIST_INDEX(i, list_size)
				une_node_free(list[i], free_wcs);
			free(list);
			break;
		}
		
		/* Nodes. */
		default:
			une_node_free(node->content.branch.a, free_wcs);
			if (node->kind != UNE_NK_SEEK)
				une_node_free(node->content.branch.b, free_wcs);
			une_node_free(node->content.branch.c, free_wcs);
			une_node_free(node->content.branch.d, free_wcs);
			break;
	
	}
	
	/* Free une_node. */
	free(node);
}

/*
Create a une_node list buffer.
*/
une_node **une_node_list_create(size_t size)
{
	une_node **nodepp = malloc((size+1)*sizeof(*nodepp));
	verify(nodepp);
	nodepp[0] = une_node_create(UNE_NK_SIZE);
	nodepp[0]->content.value._int = (une_int)size;
	return nodepp;
}

/*
Unwrap ANY or ALL node.
*/
une_node *une_node_unwrap_any_or_all(une_node *node, une_node_kind *wrapped_as)
{
	if (node->kind != UNE_NK_ANY && node->kind != UNE_NK_ALL) {
		*wrapped_as = UNE_NK_none__;
		return node;
	}
	*wrapped_as = node->kind;
	return node->content.branch.a;
}

/*
Get node name from node kind.
*/
#ifdef UNE_DEBUG
une_static__ const wchar_t *une_node_kind_to_wcs(une_node_kind kind)
{
	assert(UNE_NODE_KIND_IS_VALID(kind));
	
	return une_node_table[kind-1];
}
#endif /* UNE_DEBUG */

/*
Returns a string representation of a une_node.
This function is not dynamic and will cause a buffer overflow
if the returned array is longer than UNE_SIZE_NODE_AS_WCS. This could
realistically happen, were this function used in a real-world
environment, but since it is only used for debugging, I'm
leaving this vulnerability in here.
*/
#ifdef UNE_DEBUG
wchar_t *une_node_to_wcs(une_node *node)
{
	/* Create output buffer. */
	wchar_t *buffer = malloc(UNE_SIZE_NODE_AS_WCS*sizeof(*buffer));
	verify(buffer);
	int buffer_len = 0;
	if (node == NULL) {
		buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"NULL");
		assert(buffer_len < UNE_SIZE_NODE_AS_WCS);
		return buffer;
	}
	buffer[0] = L'\0';
	
	assert(UNE_NODE_KIND_IS_VALID(node->kind));
	
	/* Populate output buffer. */
	switch (node->kind) {
		
		/* Lists. */
		case UNE_NK_STMTS: {
			UNE_UNPACK_NODE_LIST(node, list, list_size);
			if (list_size == 0) {
				buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET "NO STMTS");
				break;
			}
			wchar_t *node_as_wcs = une_node_to_wcs(list[1]);
			int offset = swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"{%ls", node_as_wcs);
			free(node_as_wcs);
			for (size_t i=2; i<=list_size; i++) {
				node_as_wcs = une_node_to_wcs(list[i]);
				offset += swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"\n%ls", node_as_wcs);
				assert(buffer_len + offset < UNE_SIZE_NODE_AS_WCS);
				free(node_as_wcs);
			}
			buffer_len += offset + swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"}");
			break;
		}
		case UNE_NK_OBJECT:
		case UNE_NK_LIST: {
			wchar_t open = node->kind == UNE_NK_OBJECT ? L'{' : L'[';
			wchar_t close = node->kind == UNE_NK_OBJECT ? L'}' : L']';
			UNE_UNPACK_NODE_LIST(node, list, list_size);
			if (list_size == 0) {
				buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"%c%c", open, close);
				break;
			}
			wchar_t *node_as_wcs = une_node_to_wcs(list[1]);
			int offset = swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"%c%ls", open, node_as_wcs);
			free(node_as_wcs);
			for (size_t i=2; i<=list_size; i++) {
				node_as_wcs = une_node_to_wcs(list[i]);
				offset += swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L", %ls", node_as_wcs);
				assert(buffer_len + offset < UNE_SIZE_NODE_AS_WCS);
				free(node_as_wcs);
			}
			buffer_len += offset + swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"%c", close);
			break;
		}
	
		/* Numbers. */
		case UNE_NK_INT:
		case UNE_NK_SIZE:
		case UNE_NK_ID:
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_KIND L"%ls" UNE_COLOR_RESET L":"
				UNE_COLOR_NODE_DATUM_VALUE UNE_PRINTF_UNE_INT UNE_COLOR_RESET,
				une_node_kind_to_wcs(node->kind), node->content.value._int);
			break;
		case UNE_NK_FLT: {
			wchar_t *flt_as_wcs = une_flt_to_wcs(node->content.value._flt);
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_KIND L"%ls" UNE_COLOR_RESET L":"
				UNE_COLOR_NODE_DATUM_VALUE L"%ls" UNE_COLOR_RESET,
				une_node_kind_to_wcs(node->kind), flt_as_wcs);
			free(flt_as_wcs);
			break;
		}
		
		/* Strings. */
		case UNE_NK_STR:
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_KIND L"%ls" UNE_COLOR_RESET L":"
				UNE_COLOR_NODE_DATUM_VALUE L"\"%ls" UNE_COLOR_NODE_DATUM_VALUE L"\"" UNE_COLOR_RESET,
				une_node_kind_to_wcs(node->kind), node->content.value._wcs);
			break;
		case UNE_NK_NAME:
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_KIND L"%ls" UNE_COLOR_RESET L":"
				UNE_COLOR_NODE_DATUM_VALUE L"%ls" UNE_COLOR_RESET,
				une_node_kind_to_wcs(node->kind), node->content.value._wcs);
			break;
		
		/* Native functions. */
		case UNE_NK_NATIVE:
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_KIND L"%ls" UNE_COLOR_RESET L":"
				UNE_COLOR_NODE_DATUM_VALUE L"%ls" UNE_COLOR_RESET,
				une_node_kind_to_wcs(node->kind), une_native_to_wcs((une_native)node->content.value._int));
			break;
		
		/* Nullary operations/Void/this. */
		case UNE_NK_VOID:
		case UNE_NK_THIS:
		case UNE_NK_BREAK:
		case UNE_NK_CONTINUE: {
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_BRANCH_KIND L"%ls" UNE_COLOR_RESET,
				une_node_kind_to_wcs(node->kind));
			break;
		}
		
		/* Unary operations. */
		case UNE_NK_NEG:
		case UNE_NK_NOT:
		case UNE_NK_RETURN:
		case UNE_NK_ASSERT:
		case UNE_NK_ANY:
		case UNE_NK_ALL:
		case UNE_NK_EXIT: {
			wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"(" UNE_COLOR_NODE_BRANCH_KIND L"%ls"
				UNE_COLOR_RESET L" %ls" UNE_COLOR_RESET L")", une_node_kind_to_wcs(node->kind), branch1);
			free(branch1);
			break;
		}

		/* Binary operations. */
		case UNE_NK_ADD:
		case UNE_NK_SUB:
		case UNE_NK_MUL:
		case UNE_NK_DIV:
		case UNE_NK_FDIV:
		case UNE_NK_MOD:
		case UNE_NK_POW:
		case UNE_NK_MEMBER_SEEK:
		case UNE_NK_ASSIGN:
		case UNE_NK_ASSIGNADD:
		case UNE_NK_ASSIGNSUB:
		case UNE_NK_ASSIGNPOW:
		case UNE_NK_ASSIGNMUL:
		case UNE_NK_ASSIGNFDIV:
		case UNE_NK_ASSIGNDIV:
		case UNE_NK_ASSIGNMOD:
		case UNE_NK_EQU:
		case UNE_NK_NEQ:
		case UNE_NK_GTR:
		case UNE_NK_GEQ:
		case UNE_NK_LSS:
		case UNE_NK_LEQ:
		case UNE_NK_AND:
		case UNE_NK_OR:
		case UNE_NK_CALL:
		case UNE_NK_WHILE:
		case UNE_NK_COVER:
		case UNE_NK_CONCATENATE:
		case UNE_NK_OBJECT_ASSOCIATION:
		case UNE_NK_FUNCTION: {
			wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
			wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"(" UNE_COLOR_NODE_BRANCH_KIND L"%ls"
				UNE_COLOR_RESET L" %ls" UNE_COLOR_RESET L", %ls" UNE_COLOR_RESET L")",
				une_node_kind_to_wcs(node->kind), branch1, branch2);
			free(branch1);
			free(branch2);
			break;
		}
		case UNE_NK_SEEK: {
			wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"(" UNE_COLOR_NODE_BRANCH_KIND L"%ls"
				UNE_COLOR_RESET " %ls" UNE_COLOR_RESET L", GLOBAL:%d)",
				une_node_kind_to_wcs(node->kind), branch1, ((bool)node->content.branch.b == true) ? 1 : 0);
			free(branch1);
			break;
		}
		
		/* Ternary operations. */
		case UNE_NK_COP:
		case UNE_NK_FOR_ELEMENT:
		case UNE_NK_IDX_SEEK:
		case UNE_NK_IF: {
			wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
			wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
			wchar_t *branch3 = une_node_to_wcs(node->content.branch.c);
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"(" UNE_COLOR_NODE_BRANCH_KIND L"%ls"
				UNE_COLOR_RESET L" %ls" UNE_COLOR_RESET L", %ls" UNE_COLOR_RESET L", %ls" UNE_COLOR_RESET L")",
				une_node_kind_to_wcs(node->kind), branch1, branch2, branch3);
			free(branch1);
			free(branch2);
			free(branch3);
			break;
		}
		
		/* Quaternary operations. */
		case UNE_NK_FOR_RANGE: {
			wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
			wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
			wchar_t *branch3 = une_node_to_wcs(node->content.branch.c);
			wchar_t *branch4 = une_node_to_wcs(node->content.branch.d);
			buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_RESET L"(" UNE_COLOR_NODE_BRANCH_KIND L"%ls"
				UNE_COLOR_RESET L" %ls" UNE_COLOR_RESET L", %ls" UNE_COLOR_RESET L", %ls" UNE_COLOR_RESET L", %ls"
				UNE_COLOR_RESET L")", une_node_kind_to_wcs(node->kind), branch1, branch2, branch3, branch4);
			free(branch1);
			free(branch2);
			free(branch3);
			free(branch4);
			break;
		}

		default:
			assert(false);
	
	}
	
	assert(buffer_len < UNE_SIZE_NODE_AS_WCS);
	return buffer;
}
#endif /* UNE_DEBUG */
