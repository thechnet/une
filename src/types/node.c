/*
node.c - Une
Modified 2023-06-19
*/

/* Header-specific includes. */
#include "node.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "../builtin_functions.h"

/*
Node name table.
*/
const wchar_t *une_node_table[] = {
  L"ID",
  L"SIZE",
  L"VOID",
  L"INT",
  L"FLT",
  L"STR",
  L"LIST",
  L"OBJECT",
  L"FUNCTION",
  L"BUILTIN",
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
une_node *une_node_create(une_node_type type)
{
  /* Allocate new une_node. */
  une_node *node = malloc(sizeof(*node));
  verify(node);
  
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
    case UNE_NT_VOID:
    case UNE_NT_INT:
    case UNE_NT_FLT:
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE:
    case UNE_NT_SIZE:
    case UNE_NT_BUILTIN:
    case UNE_NT_THIS:
      dest->content.value = src->content.value;
      break;
    
    /* Heap data. */
    case UNE_NT_STR:
    case UNE_NT_ID:
      dest->content.value._wcs = wcsdup(src->content.value._wcs);
      verify(dest->content.value._wcs);
      break;
    
    /* Node lists. */
    case UNE_NT_LIST:
    case UNE_NT_OBJECT:
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
      if (src->type == UNE_NT_SEEK)
        dest->content.branch.b = src->content.branch.b;
      else
        dest->content.branch.b = une_node_copy(src->content.branch.b);
      if (src->type == UNE_NT_FUNCTION)
        dest->content.branch.c = src->content.branch.c;
      else
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
  assert(UNE_NODE_TYPE_IS_VALID(node->type));
  
  /* Free une_node members. */
  switch (node->type) {
    
    /* Stack data. */
    case UNE_NT_VOID:
    case UNE_NT_INT:
    case UNE_NT_FLT:
    case UNE_NT_BUILTIN:
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE:
    case UNE_NT_SIZE:
    case UNE_NT_THIS:
      break;
    
    /* Heap data. */
    case UNE_NT_STR:
    case UNE_NT_ID:
      /* DOC: Memory Management
      We don't normally free the WCS pointers because they are pointing at data stored in the tokens,
      which may still be needed after the parse. This memory is freed alongside the tokens. */
      if (free_wcs)
        free(node->content.value._wcs);
      break;
    
    /* Node lists. */
    case UNE_NT_LIST:
    case UNE_NT_OBJECT:
    case UNE_NT_STMTS: {
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
      if (node->type != UNE_NT_SEEK)
        une_node_free(node->content.branch.b, free_wcs);
      if (node->type != UNE_NT_FUNCTION)
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
  nodepp[0] = une_node_create(UNE_NT_SIZE);
  nodepp[0]->content.value._int = (une_int)size;
  return nodepp;
}

/*
Unwrap ANY or ALL node.
*/
une_node *une_node_unwrap_any_or_all(une_node *node, une_node_type *wrapped_as)
{
  if (node->type != UNE_NT_ANY && node->type != UNE_NT_ALL) {
    *wrapped_as = UNE_NT_none__;
    return node;
  }
  *wrapped_as = node->type;
  return node->content.branch.a;
}

/*
Get node name from node type.
*/
#ifdef UNE_DEBUG
une_static__ const wchar_t *une_node_type_to_wcs(une_node_type type)
{
  assert(UNE_NODE_TYPE_IS_VALID(type));
  
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
  wchar_t *buffer = malloc(UNE_SIZE_NODE_AS_WCS*sizeof(*buffer));
  verify(buffer);
  int buffer_len = 0;
  if (node == NULL) {
    buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET L"NULL");
    assert(buffer_len < UNE_SIZE_NODE_AS_WCS);
    return buffer;
  }
  buffer[0] = L'\0';
  
  assert(UNE_NODE_TYPE_IS_VALID(node->type));
  
  /* Populate output buffer. */
  switch (node->type) {
    
    /* Lists. */
    case UNE_NT_STMTS: {
      UNE_UNPACK_NODE_LIST(node, list, list_size);
      if (list_size == 0) {
        buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET "NO STMTS");
        break;
      }
      wchar_t *node_as_wcs = une_node_to_wcs(list[1]);
      int offset = swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET L"{%ls", node_as_wcs);
      free(node_as_wcs);
      for (size_t i=2; i<=list_size; i++) {
        node_as_wcs = une_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, RESET L"\n%ls", node_as_wcs);
        free(node_as_wcs);
      }
      buffer_len += offset + swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, RESET L"}");
      break;
    }
    case UNE_NT_OBJECT:
    case UNE_NT_LIST: {
      wchar_t open = node->type == UNE_NT_OBJECT ? L'{' : L'[';
      wchar_t close = node->type == UNE_NT_OBJECT ? L'}' : L']';
      UNE_UNPACK_NODE_LIST(node, list, list_size);
      if (list_size == 0) {
        buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET L"%c%c", open, close);
        break;
      }
      wchar_t *node_as_wcs = une_node_to_wcs(list[1]);
      int offset = swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET L"%c%ls", open, node_as_wcs);
      free(node_as_wcs);
      for (size_t i=2; i<=list_size; i++) {
        node_as_wcs = une_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, RESET L", %ls", node_as_wcs);
        free(node_as_wcs);
      }
      buffer_len += offset + swprintf(buffer+offset, UNE_SIZE_NODE_AS_WCS, RESET L"%c", close);
      break;
    }
  
    /* Numbers. */
    case UNE_NT_INT:
    case UNE_NT_SIZE:
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_TYPE L"%ls" RESET L":"
        UNE_COLOR_NODE_DATUM_VALUE UNE_PRINTF_UNE_INT RESET,
        une_node_type_to_wcs(node->type), node->content.value._int);
      break;
    case UNE_NT_FLT:
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_TYPE L"%ls" RESET L":"
        UNE_COLOR_NODE_DATUM_VALUE UNE_PRINTF_UNE_FLT RESET,
        une_node_type_to_wcs(node->type), node->content.value._flt);
      break;
    
    /* Strings. */
    case UNE_NT_STR:
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_TYPE L"%ls" RESET L":"
        UNE_COLOR_NODE_DATUM_VALUE L"\"%ls" UNE_COLOR_NODE_DATUM_VALUE L"\"" RESET,
        une_node_type_to_wcs(node->type), node->content.value._wcs);
      break;
    case UNE_NT_ID:
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_TYPE L"%ls" RESET L":"
        UNE_COLOR_NODE_DATUM_VALUE L"%ls" RESET,
        une_node_type_to_wcs(node->type), node->content.value._wcs);
      break;
    
    /* Built-in functions. */
    case UNE_NT_BUILTIN:
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_DATUM_TYPE L"%ls" RESET L":"
        UNE_COLOR_NODE_DATUM_VALUE L"%ls" RESET,
        une_node_type_to_wcs(node->type), une_builtin_function_to_wcs((une_builtin_function)node->content.value._int));
      break;
    
    /* Nullary operations/Void/this. */
    case UNE_NT_VOID:
    case UNE_NT_THIS:
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE: {
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, UNE_COLOR_NODE_BRANCH_TYPE L"%ls" RESET,
        une_node_type_to_wcs(node->type));
      break;
    }
    
    /* Unary operations. */
    case UNE_NT_NEG:
    case UNE_NT_NOT:
    case UNE_NT_RETURN:
    case UNE_NT_ASSERT:
    case UNE_NT_ANY:
    case UNE_NT_ALL:
    case UNE_NT_EXIT: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
        RESET L" %ls" RESET L")", une_node_type_to_wcs(node->type), branch1);
      free(branch1);
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
    case UNE_NT_MEMBER_SEEK:
    case UNE_NT_ASSIGN:
    case UNE_NT_ASSIGNADD:
    case UNE_NT_ASSIGNSUB:
    case UNE_NT_ASSIGNPOW:
    case UNE_NT_ASSIGNMUL:
    case UNE_NT_ASSIGNFDIV:
    case UNE_NT_ASSIGNDIV:
    case UNE_NT_ASSIGNMOD:
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
    case UNE_NT_COVER:
    case UNE_NT_CONCATENATE:
    case UNE_NT_OBJECT_ASSOCIATION:
    case UNE_NT_FUNCTION: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
        RESET L" %ls" RESET L", %ls" RESET L")",
        une_node_type_to_wcs(node->type), branch1, branch2);
      free(branch1);
      free(branch2);
      break;
    }
    case UNE_NT_SEEK: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
        RESET " %ls" RESET L", GLOBAL:%d)",
        une_node_type_to_wcs(node->type), branch1, ((bool)node->content.branch.b == true) ? 1 : 0);
      free(branch1);
      break;
    }
    
    /* Ternary operations. */
    case UNE_NT_COP:
    case UNE_NT_FOR_ELEMENT:
    case UNE_NT_IDX_SEEK:
    case UNE_NT_IF: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
      wchar_t *branch3 = une_node_to_wcs(node->content.branch.c);
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
        RESET L" %ls" RESET L", %ls" RESET L", %ls" RESET L")",
        une_node_type_to_wcs(node->type), branch1, branch2, branch3);
      free(branch1);
      free(branch2);
      free(branch3);
      break;
    }
    
    /* Quaternary operations. */
    case UNE_NT_FOR_RANGE: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
      wchar_t *branch3 = une_node_to_wcs(node->content.branch.c);
      wchar_t *branch4 = une_node_to_wcs(node->content.branch.d);
      buffer_len += swprintf(buffer, UNE_SIZE_NODE_AS_WCS, RESET L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
        RESET L" %ls" RESET L", %ls" RESET L", %ls" RESET L", %ls"
        RESET L")", une_node_type_to_wcs(node->type), branch1, branch2, branch3, branch4);
      free(branch1);
      free(branch2);
      free(branch3);
      free(branch4);
      break;
    }
  
  }
  
  assert(buffer_len < UNE_SIZE_NODE_AS_WCS);
  return buffer;
}
#endif /* UNE_DEBUG */
