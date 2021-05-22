/*
node.c - Une
Updated 2021-05-22
*/

#include "node.h"

#pragma region Node Name Table
wchar_t *une_node_table[] = {
  L"INT",
  L"FLT",
  L"STR",
  L"ID",
  L"LIST",
  L"SIZE",
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
  L"RETURN"
};
#pragma endregion Node Name Table

#pragma region une_node_type_to_wcs
#ifdef UNE_DEBUG
const wchar_t *une_node_type_to_wcs(une_node_type type)
{
  if (type <= 0 || type >= __UNE_NT_max__) WERR(L"Node type out of bounds: %d", type);
  return une_node_table[type-1];
}
#endif /* UNE_DEBUG */
#pragma endregion une_node_type_to_wcs

#pragma region une_node_to_wcs
#ifdef UNE_DEBUG
/*
This function is not dynamic and will cause a buffer overflow
if the returned array is longer than UNE_SIZE_MEDIUM. This could
realistically happen, were this function used in a real-world
environment, but since it is only used for debugging, I'm
leaving this vulnerability in here.
*/
wchar_t *une_node_to_wcs(une_node *node)
{
  wchar_t *buffer = rmalloc(UNE_SIZE_BIG * sizeof(*buffer));
  if (node == NULL) {
    wcscpy(buffer, UNE_COLOR_HINT L"NULL" UNE_COLOR_NEUTRAL);
    return buffer;
  }
  buffer[0] = L'\0';
  switch (node->type) {
    
    #pragma region STMTS
    case UNE_NT_STMTS: {
      une_node **list = (une_node**)node->content.value._vpp;
      if (list == NULL) WERR(L"Undefined stmts pointer");
      size_t list_size = list[0]->content.value._int;
      if (list_size == 0) {
        swprintf(buffer, UNE_SIZE_BIG,
                 UNE_COLOR_HINT L"NO STMTS" UNE_COLOR_NEUTRAL);
        break;
      }
      wchar_t *node_as_wcs = une_node_to_wcs(list[1]);
      size_t offset = swprintf(buffer, UNE_SIZE_BIG,
                               UNE_COLOR_NEUTRAL L"{%ls",
                               node_as_wcs);
      free(node_as_wcs);
      for (size_t i=2; i<=list_size; i++) {
        node_as_wcs = une_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, UNE_SIZE_BIG,
                           UNE_COLOR_NEUTRAL L"\n%ls",
                           node_as_wcs);
        free(node_as_wcs);
      }
      swprintf(buffer+offset, UNE_SIZE_BIG, UNE_COLOR_NEUTRAL L"}");
      break; }
    #pragma endregion STMTS
    
    #pragma region DATA
    case UNE_NT_INT:
    case UNE_NT_SIZE:
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NODE_DATUM_TYPE L"%ls" UNE_COLOR_NEUTRAL L":"
               UNE_COLOR_NODE_DATUM_VALUE L"%lld" UNE_COLOR_NEUTRAL,
               une_node_type_to_wcs(node->type),
               node->content.value._int);
      break;
    
    case UNE_NT_FLT:
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NODE_DATUM_TYPE L"%ls" UNE_COLOR_NEUTRAL L":"
               UNE_COLOR_NODE_DATUM_VALUE L"%.3f" UNE_COLOR_NEUTRAL,
               une_node_type_to_wcs(node->type),
               node->content.value._flt);
      break;
    
    case UNE_NT_STR:
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NODE_DATUM_TYPE L"%ls" UNE_COLOR_NEUTRAL L":"
               UNE_COLOR_NODE_DATUM_VALUE L"\"%ls"
               UNE_COLOR_NODE_DATUM_VALUE L"\"" UNE_COLOR_NEUTRAL,
               une_node_type_to_wcs(node->type),
               node->content.value._wcs);
      break;
    
    case UNE_NT_ID:
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NODE_DATUM_TYPE L"%ls" UNE_COLOR_NEUTRAL L":"
               UNE_COLOR_NODE_DATUM_VALUE L"%ls" UNE_COLOR_NEUTRAL,
               une_node_type_to_wcs(node->type),
               node->content.value._wcs);
      break;
    
    case UNE_NT_LIST: {
      une_node **list = (une_node**)node->content.value._vpp;
      if (list == NULL) WERR(L"Undefined list pointer");
      size_t list_size = list[0]->content.value._int;
      if (list_size == 0) {
        swprintf(buffer, UNE_SIZE_BIG, UNE_COLOR_NEUTRAL L"[]");
        break;
      }
      wchar_t *node_as_wcs = une_node_to_wcs(list[1]);
      int offset = swprintf(buffer, UNE_SIZE_BIG,
                            UNE_COLOR_NEUTRAL L"[%ls",
                            node_as_wcs);
      free(node_as_wcs);
      for (int i=2; i<=list_size; i++) {
        node_as_wcs = une_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, UNE_SIZE_BIG,
                           UNE_COLOR_NEUTRAL L", %ls",
                           node_as_wcs);
        free(node_as_wcs);
      }
      swprintf(buffer+offset, UNE_SIZE_BIG, UNE_COLOR_NEUTRAL L"]");
      break; }
    #pragma endregion DATA

    #pragma region Nullary Operations
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE: {
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NODE_BRANCH_TYPE L"%ls" UNE_COLOR_NEUTRAL,
               une_node_type_to_wcs(node->type));
      break; }
    #pragma endregion Nullary Operations
    
    #pragma region Unary Operations
    case UNE_NT_NEG:
    case UNE_NT_NOT:
    case UNE_NT_GET:
    case UNE_NT_RETURN: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L")",
               une_node_type_to_wcs(node->type),
               branch1);
      free(branch1);
      break; }
    #pragma endregion Unary Operations

    #pragma region Binary Operations
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
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L")",
               une_node_type_to_wcs(node->type),
               branch1,
               branch2);
      free(branch1);
      free(branch2);
      break; }
    #pragma endregion Binary Operation
    
    #pragma region Ternary Operations
    case UNE_NT_DEF:
    case UNE_NT_SET_IDX:
    case UNE_NT_COP:
    case UNE_NT_IF: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
      wchar_t *branch3 = une_node_to_wcs(node->content.branch.c);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L")",
               une_node_type_to_wcs(node->type),
               branch1,
               branch2,
               branch3);
      free(branch1);
      free(branch2);
      free(branch3);
      break; }
    #pragma endregion Ternary Operations
    
    #pragma region Quaternary Operations
    case UNE_NT_FOR: {
      wchar_t *branch1 = une_node_to_wcs(node->content.branch.a);
      wchar_t *branch2 = une_node_to_wcs(node->content.branch.b);
      wchar_t *branch3 = une_node_to_wcs(node->content.branch.c);
      wchar_t *branch4 = une_node_to_wcs(node->content.branch.d);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L")",
               une_node_type_to_wcs(node->type),
               branch1,
               branch2,
               branch3,
               branch4);
      free(branch1);
      free(branch2);
      free(branch3);
      free(branch4);
      break; }
    #pragma endregion Quaternary Operations

    default:
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(UNKNOWN NODE TYPE OR MEMORY)");
  }
  return buffer;
}
#endif /* UNE_DEBUG */
#pragma endregion une_node_to_wcs

#pragma region une_node_free
void une_node_free(une_node *node, bool free_wcs)
{
  if (node == NULL) {
    #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
      wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: NULL\n", __FILE__, __FUNCTION__, __LINE__);
    #endif
    return;
  }

  switch (node->type) {
    
    #pragma region Stack / Nullary Operation
    case UNE_NT_INT:
    case UNE_NT_FLT:
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE:
    case UNE_NT_SIZE:
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    #pragma endregion Stack / Nullary Operation
      
    #pragma region String
    case UNE_NT_STR:
    case UNE_NT_ID:
      /* DOC: Memory Management
      We don't normally free the WCS pointers because they are pointing at data stored in the tokens,
      which may still be needed after the parse. This memory is freed alongside the tokens.
      */
      if (free_wcs) free(node->content.value._wcs);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    #pragma endregion String
    
    #pragma region Sequence
    case UNE_NT_LIST:
    case UNE_NT_STMTS: {
      une_node **list = (une_node**)node->content.value._vpp;
      if (list == NULL) {
        // wprintf(L"Warning: Node to be freed has undefined list pointer.\n");
        break;
      }
      size_t list_size = list[0]->content.value._int;
      for (size_t i=0; i<=list_size; i++) {
        une_node_free(list[i], free_wcs);
      }
      free(list);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break; }
    #pragma endregion Sequence
    
    #pragma region Unary Operation
    case UNE_NT_NEG:
    case UNE_NT_NOT:
    case UNE_NT_GET:
    case UNE_NT_RETURN:
      une_node_free(node->content.branch.a, free_wcs);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    #pragma endregion Unary Operation
    
    #pragma region Binary Operation
    case UNE_NT_POW:
    case UNE_NT_MUL:
    case UNE_NT_DIV:
    case UNE_NT_FDIV:
    case UNE_NT_MOD:
    case UNE_NT_ADD:
    case UNE_NT_SUB:
    case UNE_NT_EQU:
    case UNE_NT_NEQ:
    case UNE_NT_GTR:
    case UNE_NT_GEQ:
    case UNE_NT_LSS:
    case UNE_NT_LEQ:
    case UNE_NT_AND:
    case UNE_NT_OR:
    case UNE_NT_SET:
    case UNE_NT_CALL:
    case UNE_NT_WHILE:
    case UNE_NT_GET_IDX:
      une_node_free(node->content.branch.a, free_wcs);
      une_node_free(node->content.branch.b, free_wcs);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    #pragma endregion Binary Operation
    
    #pragma region Ternary Operation
    case UNE_NT_COP:
    case UNE_NT_DEF:
    case UNE_NT_IF:
    case UNE_NT_SET_IDX:
      une_node_free(node->content.branch.a, free_wcs);
      une_node_free(node->content.branch.b, free_wcs);
      une_node_free(node->content.branch.c, free_wcs);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    #pragma endregion Ternary Operation
    
    #pragma region Quaternary Operation
    case UNE_NT_FOR:
      une_node_free(node->content.branch.a, free_wcs);
      une_node_free(node->content.branch.b, free_wcs);
      une_node_free(node->content.branch.c, free_wcs);
      une_node_free(node->content.branch.d, free_wcs);
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    #pragma endregion Quaternary Operation
    
    default:
      WERR(L"Unhandled node type!\n");
  }
}
#pragma endregion une_node_free

#pragma region une_node_create
une_node *une_node_create(une_node_type type)
{
  une_node *node = rmalloc(sizeof(*node));
  node->type = type;
  node->pos = (une_position){0, 0};
  node->content.branch.a = NULL;
  node->content.branch.b = NULL;
  node->content.branch.c = NULL;
  node->content.branch.d = NULL;
  return node;
}
#pragma endregion une_node_create

#pragma region une_node_copy
une_node *une_node_copy(une_node *src)
{
  if(src == NULL) return NULL;
  une_node *dest = une_node_create(src->type);
  dest->pos = src->pos;
  switch(src->type) {
    case UNE_NT_INT:
    case UNE_NT_SIZE:
    case UNE_NT_FLT:
      dest->content.value = src->content.value;
      break;
    
    case UNE_NT_STR:
    case UNE_NT_ID:
      dest->content.value._wcs = wcs_dup(src->content.value._wcs);
      break;
    
    case UNE_NT_LIST:
    case UNE_NT_STMTS: {
      une_node **list = (une_node**)src->content.value._vpp;
      size_t size = list[0]->content.value._int;
      une_node **new_list = rmalloc((size+1)*sizeof(*new_list));
      for(size_t i=0; i<=size; i++) new_list[i] = une_node_copy(list[i]);
      dest->content.value._vpp = (void**)new_list;
      break; }

    default:
      dest->content.branch.a = une_node_copy(src->content.branch.a);
      dest->content.branch.b = une_node_copy(src->content.branch.b);
      dest->content.branch.c = une_node_copy(src->content.branch.c);
      dest->content.branch.d = une_node_copy(src->content.branch.d);
      break;
  }
  return dest;
}
#pragma endregion une_node_copy
