/*
node.c - Une
Modified 2021-07-05
*/

/* Header-specific includes. */
#include "node.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Node name table.
*/
const wchar_t *une_node_table[] = {
  L"ID",
  L"SIZE",
  L"INT",
  L"FLT",
  L"STR",
  L"LIST",
  L"STMTS",
  L"COP",
  L"NOT",
  L"AND",
  L"OR",
  L"EQU",
  L"NEQ",
  L"GTR",
  L"GEQ",
  L"LSS",
  L"LEQ",
  L"ADD",
  L"SUB",
  L"MUL",
  L"DIV",
  L"FDIV",
  L"MOD",
  L"POW",
  L"NEG",
  L"SET",
  L"SET_IDX",
  L"GET",
  L"GET_IDX",
  L"DEF",
  L"CALL",
  L"FOR",
  L"WHILE",
  L"IF",
  L"CONTINUE",
  L"BREAK",
  L"RETURN",
};

/*
Allocate, initialize, and return a une_node struct pointer.
*/
une_node *une_node_create(une_node_type type)
{
  /* Allocate new une_node. */
  une_node *node = une_malloc(sizeof(*node));
  
  /* Initialize new une_node. */
  *node = (une_node){
    .type = type,
    .pos = (une_position){0, 0},
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
  une_node *dest = une_node_create(src->type);
  
  /* Populate destination une_node. */
  dest->pos = src->pos;
  switch (src->type) {
    
    /* Stack data. */
    case UNE_NT_INT:
    case UNE_NT_FLT:
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE:
    case UNE_NT_SIZE:
      dest->content.value = src->content.value;
      break;
    
    /* Heap data. */
    case UNE_NT_STR:
    case UNE_NT_ID:
      dest->content.value._wcs = une_wcsdup(src->content.value._wcs);
      break;
    
    /* List. */
    case UNE_NT_LIST:
    case UNE_NT_STMTS: {
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
      dest->content.branch.b = une_node_copy(src->content.branch.b);
      dest->content.branch.c = une_node_copy(src->content.branch.c);
      if (src->type == UNE_NT_SET || src->type == UNE_NT_SET_IDX)
        dest->content.branch.d = src->content.branch.d;
      else
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
  /* Ensure une_node exists. */
  if (node == NULL)
    return;
  
  /* Free une_node members. */
  switch (node->type) {
    
    /* Stack data. */
    case UNE_NT_INT:
    case UNE_NT_FLT:
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE:
    case UNE_NT_SIZE:
      LOGFREE(L"une_node", une_node_type_to_wcs(node->type), node->type);
      break;
    
    /* Heap data. */
    case UNE_NT_STR:
    case UNE_NT_ID:
      /* DOC: Memory Management
      We don't normally free the WCS pointers because they are pointing at data stored in the tokens,
      which may still be needed after the parse. This memory is freed alongside the tokens. */
      if (free_wcs)
        une_free(node->content.value._wcs);
      LOGFREE(L"une_node", une_node_type_to_wcs(node->type), node->type);
      break;
    
    /* List. */
    case UNE_NT_LIST:
    case UNE_NT_STMTS: {
      une_node **list = (une_node**)node->content.value._vpp;
      if (list == NULL)
        break;
      size_t list_size = list[0]->content.value._int;
      UNE_FOR_NODE_LIST_INDEX(i, list_size)
        une_node_free(list[i], free_wcs);
      une_free(list);
      LOGFREE(L"une_node", une_node_type_to_wcs(node->type), node->type);
      break;
    }
    
    /* Nodes. */
    default:
      une_node_free(node->content.branch.a, free_wcs);
      une_node_free(node->content.branch.b, free_wcs);
      une_node_free(node->content.branch.c, free_wcs);
      if (node->type != UNE_NT_SET && node->type != UNE_NT_SET_IDX)
        une_node_free(node->content.branch.d, free_wcs);
      LOGFREE(L"une_node", une_node_type_to_wcs(node->type), node->type);
      break;
  
  }
  
  /* Free une_node. */
  une_free(node);
}

/*
Create a une_node list buffer.
*/
une_node **une_node_list_create(size_t size)
{
  une_node **nodepp = une_malloc((size+1)*sizeof(*nodepp));
  nodepp[0] = une_node_create(UNE_NT_SIZE);
  nodepp[0]->content.value._int = size;
  return nodepp;
}

/*
Get node name from node type.
*/
#ifdef UNE_DEBUG
/*__une_static*/ const wchar_t *une_node_type_to_wcs(une_node_type type)
{
  /* Ensure une_node_type is valid. */
  UNE_VERIFY_NODE_TYPE(type);
  
  return une_node_table[type-1];
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
  wchar_t *buffer = une_malloc(UNE_SIZE_NODE_AS_WCS*sizeof(*buffer));
  if (node == NULL) {
    wcscpy(buffer, UNE_COLOR_HINT L"NULL" UNE_COLOR_NEUTRAL);
    return buffer;
  }
  buffer[0] = L'\0';
  
  /* Ensure une_node_type is valid. */
  UNE_VERIFY_NODE_TYPE(node->type);
  
  /* Populate output buffer. */
  switch (node->type) {
    
    /* Lists. */
    case UNE_NT_STMTS: {
      UNE_UNPACK_NODE_LIST(node, list, list_size);
      if (list_size == 0) {
        swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_HINT L"NO STMTS" UNE_COLOR_NEUTRAL);
        break;
      }
      wchar_t *node_as_wcs = une_node_to_wcs(list[1]);
      size_t offset = swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"{%ls", node_as_wcs);
      une_free(node_as_wcs);
      for (size_t i=2; i<=list_size; i++) {
        node_as_wcs = une_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"\n%ls", node_as_wcs);
        une_free(node_as_wcs);
      }
      swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"}");
      break;
    }
    case UNE_NT_LIST: {
      UNE_UNPACK_NODE_LIST(node, list, list_size);
      if (list_size == 0) {
        swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"[]");
        break;
      }
      wchar_t *node_as_wcs = une_node_to_wcs(list[1]);
      int offset = swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"[%ls", node_as_wcs);
      une_free(node_as_wcs);
      for (int i=2; i<=list_size; i++) {
        node_as_wcs = une_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L", %ls", node_as_wcs);
        une_free(node_as_wcs);
      }
      swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"]");
      break;
    }
  
    /* Numbers. */
    case UNE_NT_INT:
    case UNE_NT_SIZE:
      swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_TYPE L"%ls" UNE_COLOR_NEUTRAL L":"
        UNE_COLOR_NODE_DATUM_VALUE L"%lld" UNE_COLOR_NEUTRAL,
        une_node_type_to_wcs(node->type), node->content.value._int);
      break;
    case UNE_NT_FLT:
      swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_TYPE L"%ls" UNE_COLOR_NEUTRAL L":"
        UNE_COLOR_NODE_DATUM_VALUE L"%.3f" UNE_COLOR_NEUTRAL,
        une_node_type_to_wcs(node->type), node->content.value._flt);
      break;
    
    /* Strings. */
    case UNE_NT_STR:
      swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_TYPE L"%ls" UNE_COLOR_NEUTRAL L":"
        UNE_COLOR_NODE_DATUM_VALUE L"\"%ls" UNE_COLOR_NODE_DATUM_VALUE L"\"" UNE_COLOR_NEUTRAL,
        une_node_type_to_wcs(node->type), node->content.value._wcs);
      break;
    case UNE_NT_ID:
      swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_TYPE L"%ls" UNE_COLOR_NEUTRAL L":"
        UNE_COLOR_NODE_DATUM_VALUE L"%ls" UNE_COLOR_NEUTRAL,
        une_node_type_to_wcs(node->type), node->content.value._wcs);
      break;
    
    /* Nullary operations. */
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE: {
      swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_BRANCH_TYPE L"%ls" UNE_COLOR_NEUTRAL,
        une_node_type_to_wcs(node->type));
      break;
    }
    
    /* Unary operations. */
    case UNE_NT_NEG:
    case UNE_NT_NOT:
    case UNE_NT_GET:
    case UNE_NT_RETURN: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
        UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L")", une_node_type_to_wcs(node->type), branch1);
      une_free(branch1);
      break;
    }

    /* Binary operations. */
    case UNE_NT_ADD:
    case UNE_NT_SUB:
    case UNE_NT_MUL:
    case UNE_NT_DIV:
    case UNE_NT_FDIV:
    case UNE_NT_MOD:
    case UNE_NT_POW:
    case UNE_NT_GET_IDX:
    case UNE_NT_EQU:
    case UNE_NT_NEQ:
    case UNE_NT_GTR:
    case UNE_NT_GEQ:
    case UNE_NT_LSS:
    case UNE_NT_LEQ:
    case UNE_NT_AND:
    case UNE_NT_OR:
    case UNE_NT_CALL:
    case UNE_NT_WHILE:
    case UNE_NT_SET: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
      swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
        UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L")",
        une_node_type_to_wcs(node->type), branch1, branch2);
      une_free(branch1);
      une_free(branch2);
      break;
    }
    
    /* Ternary operations. */
    case UNE_NT_DEF:
    case UNE_NT_SET_IDX:
    case UNE_NT_COP:
    case UNE_NT_IF: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
      wchar_t *branch3 = une_node_to_wcs(node->content.branch.c);
      swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
        UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L")",
        une_node_type_to_wcs(node->type), branch1, branch2, branch3);
      une_free(branch1);
      une_free(branch2);
      une_free(branch3);
      break;
    }
    
    /* Quaternary operations. */
    case UNE_NT_FOR: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
      wchar_t *branch3 = une_node_to_wcs(node->content.branch.c);
      wchar_t *branch4 = une_node_to_wcs(node->content.branch.d);
      swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
        UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L", %ls"
        UNE_COLOR_NEUTRAL L")", une_node_type_to_wcs(node->type), branch1, branch2, branch3, branch4);
      une_free(branch1);
      une_free(branch2);
      une_free(branch3);
      une_free(branch4);
      break;
    }
  
  }
  
  return buffer;
}
#endif /* UNE_DEBUG */
