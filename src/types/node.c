/*
node.c - Une
Updated 2021-04-29
*/

#include "node.h"

#pragma region une_node_type_to_wcs
const wchar_t *une_node_type_to_wcs(une_node_type type)
{
  switch (type) {
    // Data Types
    case UNE_NT_INT: return L"INT";
    case UNE_NT_FLT: return L"FLT";
    case UNE_NT_STR: return L"STR";
    case UNE_NT_LIST: return L"LIST";
    case UNE_NT_ID: return L"ID";
    // Arithmetic Operations
    case UNE_NT_POW: return L"POW";
    case UNE_NT_MUL: return L"MUL";
    case UNE_NT_DIV: return L"DIV";
    case UNE_NT_FDIV: return L"FDIV";
    case UNE_NT_MOD: return L"MOD";
    case UNE_NT_ADD: return L"ADD";
    case UNE_NT_SUB: return L"SUB";
    case UNE_NT_NEG: return L"NEG";
    // Logical Operations
    case UNE_NT_NOT: return L"NOT";
    case UNE_NT_EQU: return L"EQU";
    case UNE_NT_NEQ: return L"NEQ";
    case UNE_NT_GTR: return L"GTR";
    case UNE_NT_GEQ: return L"GEQ";
    case UNE_NT_LSS: return L"LSS";
    case UNE_NT_LEQ: return L"LEQ";
    case UNE_NT_AND: return L"AND";
    case UNE_NT_OR: return L"OR";
    // Conditional Operation
    case UNE_NT_COP: return L"COP";
    // Set, Get
    case UNE_NT_GET_IDX: return L"IDX";
    case UNE_NT_SET: return L"SET";
    case UNE_NT_SET_IDX: return L"SET_IDX";
    case UNE_NT_GET: return L"GET";
    case UNE_NT_DEF: return L"DEF";
    case UNE_NT_CALL: return L"CALL";
    // Control Flow
    case UNE_NT_FOR: return L"FOR";
    case UNE_NT_WHILE: return L"WHILE";
    case UNE_NT_IF: return L"IF";
    case UNE_NT_RETURN: return L"RETURN";
    case UNE_NT_BREAK: return L"BREAK";
    case UNE_NT_CONTINUE: return L"CONTINUE";
    case UNE_NT_STMTS: return L"STMTS";
    case UNE_NT_SIZE: return L"SIZE";
    
    default: return L"?";
  }
}
#pragma endregion une_node_type_to_wcs

#pragma region une_node_to_wcs
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

    // Binary Operation
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
    case UNE_NT_SET: {
      wchar_t *left_branch_as_wcs = une_node_to_wcs(
                                     node->content.branch.a);
      wchar_t *right_branch_as_wcs = une_node_to_wcs(
                                      node->content.branch.b);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L")",
               une_node_type_to_wcs(node->type),
               left_branch_as_wcs,
               right_branch_as_wcs);
      free(left_branch_as_wcs);
      free(right_branch_as_wcs);
      break; }
    
    // Unary Operation
    case UNE_NT_NEG:
    case UNE_NT_NOT:
    case UNE_NT_GET:
    case UNE_NT_RETURN: {
      wchar_t *center_branch_as_wcs = une_node_to_wcs(
                                       node->content.branch.a);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"%ls"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L")",
               une_node_type_to_wcs(node->type),
               center_branch_as_wcs);
      free(center_branch_as_wcs);
      break; }
    
    // Without operation
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE: {
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NODE_BRANCH_TYPE L"%ls" UNE_COLOR_NEUTRAL,
               une_node_type_to_wcs(node->type));
      break; }
    
    case UNE_NT_FOR: {
      wchar_t *counter_branch_as_wcs = une_node_to_wcs(
                                        node->content.branch.a);
      wchar_t *from_branch_as_wcs = une_node_to_wcs(
                                     node->content.branch.b);
      wchar_t *to_branch_as_wcs = une_node_to_wcs(
                                   node->content.branch.c);
      wchar_t *body_branch_as_wcs = une_node_to_wcs(
                                     node->content.branch.d);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"FOR"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L")",
               counter_branch_as_wcs,
               from_branch_as_wcs,
               to_branch_as_wcs,
               body_branch_as_wcs);
      free(counter_branch_as_wcs);
      free(from_branch_as_wcs);
      free(to_branch_as_wcs);
      free(body_branch_as_wcs);
      break; }
    
    case UNE_NT_WHILE: {
      wchar_t *condition_branch_as_wcs = une_node_to_wcs(
                                          node->content.branch.a);
      wchar_t *body_branch_as_wcs = une_node_to_wcs(
                                     node->content.branch.b);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"WHILE"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L")",
               condition_branch_as_wcs,
               body_branch_as_wcs);
      free(condition_branch_as_wcs);
      free(body_branch_as_wcs);
      break; }

    case UNE_NT_IF: {
      wchar_t *condition_branch_as_wcs = une_node_to_wcs(
                                          node->content.branch.a);
      wchar_t *truebody_branch_as_wcs = une_node_to_wcs(
                                         node->content.branch.b);
      wchar_t *falsebody_branch_as_wcs = une_node_to_wcs(
                                          node->content.branch.c);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE "IF"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L")",
               condition_branch_as_wcs,
               truebody_branch_as_wcs,
               falsebody_branch_as_wcs);
      free(condition_branch_as_wcs);
      free(truebody_branch_as_wcs);
      free(falsebody_branch_as_wcs);
      break; }
    
    case UNE_NT_COP: {
      wchar_t *trueconditions_branch_as_wcs = une_node_to_wcs(
                                               node->content.branch.a);
      wchar_t *condition_branch_as_wcs = une_node_to_wcs(
                                          node->content.branch.b);
      wchar_t *falseconditions_branch_as_wcs = une_node_to_wcs(
                                                node->content.branch.c);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"COP"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L")",
               trueconditions_branch_as_wcs,
               condition_branch_as_wcs,
               falseconditions_branch_as_wcs);
      free(trueconditions_branch_as_wcs);
      free(condition_branch_as_wcs);
      free(falseconditions_branch_as_wcs);
      break; }
    
    case UNE_NT_SET_IDX: {
      wchar_t *name_branch_as_wcs = une_node_to_wcs(
                                     node->content.branch.a);
      wchar_t *index_branch_as_wcs = une_node_to_wcs(
                                      node->content.branch.b);
      wchar_t *value_branch_as_wcs = une_node_to_wcs(
                                      node->content.branch.c);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"SET_IDX"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L")",
               name_branch_as_wcs,
               index_branch_as_wcs,
               value_branch_as_wcs);
      free(name_branch_as_wcs);
      free(index_branch_as_wcs);
      free(value_branch_as_wcs);
      break; }
    
    case UNE_NT_DEF: {
      wchar_t *name_branch_as_wcs = une_node_to_wcs(
                                     node->content.branch.a);
      wchar_t *params_branch_as_wcs = une_node_to_wcs(
                                       node->content.branch.b);
      wchar_t *body_branch_as_wcs = une_node_to_wcs(
                                     node->content.branch.c);
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(" UNE_COLOR_NODE_BRANCH_TYPE L"DEF"
               UNE_COLOR_NEUTRAL L" %ls" UNE_COLOR_NEUTRAL L", %ls"
               UNE_COLOR_NEUTRAL L", %ls" UNE_COLOR_NEUTRAL L")",
               name_branch_as_wcs,
               params_branch_as_wcs,
               body_branch_as_wcs);
      free(name_branch_as_wcs);
      free(params_branch_as_wcs);
      free(body_branch_as_wcs);
      break; }

    default:
      swprintf(buffer, UNE_SIZE_BIG,
               UNE_COLOR_NEUTRAL L"(UNKNOWN NODE TYPE OR MEMORY)");
  }
  return buffer;
}
#pragma endregion une_node_to_wcs

#pragma region une_node_free
void une_node_free(une_node *node, bool free_wcs)
{
  if (node == NULL) {
    #ifdef UNE_DEBUG_LOG_FREE
      wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: NULL\n", __FILE__, __FUNCTION__, __LINE__);
    #endif
    return;
  }

  switch (node->type) {
    // No Operation (Data)
    case UNE_NT_INT:
    case UNE_NT_FLT:
    case UNE_NT_BREAK:
    case UNE_NT_CONTINUE:
    case UNE_NT_SIZE:
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;

    // Strings
    case UNE_NT_STR:
    case UNE_NT_ID:
      /* DOC: Memory Management
      We don't normally free the WCS pointers because they are pointing at data stored in the tokens,
      which may still be needed after the parse. This memory is freed alongside the tokens.
      */
      if (free_wcs) free(node->content.value._wcs);
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    // Sequences (Data)
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
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break; }
    
    // Unary Operation
    case UNE_NT_NEG:
    case UNE_NT_NOT:
    case UNE_NT_GET:
    case UNE_NT_RETURN:
      une_node_free(node->content.branch.a, free_wcs);
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    // Binary Operation
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
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    // Ternary Operation
    case UNE_NT_COP:
    case UNE_NT_DEF:
    case UNE_NT_IF:
    case UNE_NT_SET_IDX:
      une_node_free(node->content.branch.a, free_wcs);
      une_node_free(node->content.branch.b, free_wcs);
      une_node_free(node->content.branch.c, free_wcs);
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    // Quaternary Operation
    case UNE_NT_FOR:
      une_node_free(node->content.branch.a, free_wcs);
      une_node_free(node->content.branch.b, free_wcs);
      une_node_free(node->content.branch.c, free_wcs);
      une_node_free(node->content.branch.d, free_wcs);
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Node: %ls\n", __FILE__, __FUNCTION__, __LINE__, une_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    default: WERR(L"Unhandled node type in une_node_free()!\n", free_wcs);
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
    case UNE_NT_SIZE: // FIXME: .value = .value?
      dest->content.value._int = src->content.value._int;
      break;
    
    case UNE_NT_FLT:
      dest->content.value._flt = src->content.value._flt;
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
