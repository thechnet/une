/*
parser.c - Une
Updated 2021-04-17
*/

#include "parser.h"

#pragma region une_parse_block
une_node *une_parse_block(
  une_token *tokens,
  size_t *token_index,
  une_error *error
)
{
  // {
  if(tokens[*token_index].type != UNE_TT_LBRC)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)UNE_TT_LBRC;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;

  une_node *block = une_node_create(UNE_NT_STMTS);
  block->pos.start = tokens[*token_index].pos.start;
  
  // ...
  une_node ** sequence = une_parse_sequence(
    tokens, token_index, error,
    &une_parse_stmt, UNE_TT_NEW, UNE_TT_RBRC
  );
  if(sequence == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_LIST;
      sequence = error_list;
    #else
      une_node_free(block);
      return NULL;
    #endif
  }
  block->content.value._vpp = (void**)sequence;
  
  if(sequence[0]->content.value._int == 0)
  {
    une_node_free(sequence[0]);
    free(sequence);
    error->type = UNE_ET_UNEXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)UNE_TT_RBRC;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_LIST;
      sequence = error_list;
    #else
      une_node_free(block);
      return NULL;
    #endif
  }

  // Technically redundant since une_parse_sequence already checks for this...
  // }
  if(tokens[*token_index].type != UNE_TT_RBRC)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)UNE_TT_RBRC;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      une_node_free(block);
      return NULL;
    #endif
  }

  block->pos.end = tokens[*token_index].pos.end;
  
  // ... it doesn't advance past it, though.
  (*token_index)++;

  return block;
}
#pragma endregion une_parse_block

#pragma region une_parse_sequence
/*
Unlike all other une_parse_ functions, this function does NOT return a node.
Instead it returns a list of nodes (une_node**) to be set to the datum of a
node by the parent function.
*/
une_node ** une_parse_sequence(
  une_token *tokens, size_t *token_index, une_error *error,
  une_node *(*parser)(une_token *tokens, size_t *token_index, une_error *error),
  une_token_type tt_end_of_item,
  une_token_type tt_end_of_sequence
)
{
  size_t sequence_size = UNE_SIZE_MEDIUM; // FIXME:
  une_node ** sequence = malloc(sequence_size *sizeof(*sequence));
  if(sequence == NULL) WERR(L"Out of memory.");
  
  size_t pos_start = tokens[*token_index].pos.start;

  une_node *counter = une_node_create(UNE_NT_VOID);
  sequence[0] = counter;
  
  size_t sequence_index = 1;

  while(true)
  {
    if(tokens[*token_index].type == tt_end_of_item)
    {
      (*token_index)++;
    }

    if(sequence_index >= sequence_size)
    {
      sequence_size *= 2;
      une_node ** _sequence = realloc(sequence, sequence_size *sizeof(_sequence));
      if(_sequence == NULL) WERR(L"Out of memory.");
      sequence = _sequence;
      wprintf(L"Warning: Sequence doubled\n");
    }

    // EXPECTED END OF SEQUENCE
    if(tokens[*token_index].type == tt_end_of_sequence)
    {
      /*
      We don't skip EOF because it may still be needed by
      other functions up the call chain.
      */
      //if(tt_end_of_sequence != UNE_TT_EOF) (*token_index)++;
      /*
      We don't skip the end of the sequence because:
      - If it is EOF, that may still be needed by other 
        functions up the call chain.
      - The last token's position my still be needed to
        determine the sequence's position.
      */
      break;
    }

    // UNEXPECTED END OF SEQUENCE
    if(tokens[*token_index].type == UNE_TT_EOF)
    {
      /*
      This can happen if a block, list, list of parameters, or list of arguments is opened at the end of the file without being closed.
      */
      for(size_t i=0; i<sequence_index; i++) une_node_free(sequence[i]);
      free(sequence);
      error->type = UNE_ET_UNEXPECTED_TOKEN;
      error->pos = tokens[*token_index].pos;
      error->values[0]._int = (int)UNE_TT_EOF;
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_LIST;
        return error_list;
      #else
        return NULL;
      #endif
    }

    // Get item.
    sequence[sequence_index] = (*parser)(tokens, token_index, error);
    if(sequence[sequence_index] == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        sequence[sequence_index] = error_node;
      #else
        for(size_t i=0; i<sequence_index; i++) une_node_free(sequence[i]);
        free(sequence);
        return NULL;
      #endif
    }
    sequence_index++;
    
    if(tokens[*token_index].type != tt_end_of_sequence
    && tokens[*token_index].type != tt_end_of_item)
    {
      for(size_t i=0; i<sequence_index; i++) une_node_free(sequence[i]);
      free(sequence);
      error->type = UNE_ET_EXPECTED_TOKEN;
      error->pos = tokens[*token_index].pos;
      error->values[0]._int = (int)tt_end_of_item;
      error->values[1]._int = (int)tt_end_of_sequence;
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_LIST;
        return error_list;
      #else
        return NULL;
      #endif
    }
  }

  sequence[0]->content.value._int = sequence_index-1;

  return sequence;
}
#pragma endregion une_parse_sequence

#pragma region une_parse_stmt
une_node *une_parse_stmt(
  une_token *tokens,
  size_t *token_index,
  une_error *error
  /*bool is_loop_body*/
)
{
  if(tokens[*token_index].type == UNE_TT_NEW) (*token_index)++; // FIXME: Unsure...
  
  switch(tokens[*token_index].type)
  {
    case UNE_TT_FOR: return une_parse_for(tokens, token_index, error);
    case UNE_TT_IF: return une_parse_if(tokens, token_index, error, UNE_TT_IF);
    case UNE_TT_WHILE: return une_parse_while(tokens, token_index, error);
    case UNE_TT_DEF: return une_parse_def(tokens, token_index, error);
    case UNE_TT_LBRC: return une_parse_block(tokens, token_index, error);
    case UNE_TT_RETURN: return une_parse_return(tokens, token_index, error);
    
    #pragma region Break
    case UNE_TT_BREAK: {
      // if(!is_loop_body)
      // {
      //   error->type = UNE_ET_BREAK_OUTSIDE_LOOP;
      //   error->pos = tokens[*token_index].pos;
      //   error->__line__ = __LINE__;
      //   strcpy(error->__file__, __FILE__);
      //   #ifdef UNE_DEBUG_SOFT_ERROR
      //     CREATE_ERROR_NODE;
      //     (*token_index)++;
      //     return error_node;
      //   #else
      //     return NULL;
      //   #endif
      // }
      une_node *breaknode = une_node_create(UNE_NT_BREAK);
      breaknode->pos = tokens[*token_index].pos;
      (*token_index)++;
      return breaknode; }
    #pragma endregion Break
    
    #pragma region Continue
    case UNE_TT_CONTINUE: {
      // if(!is_loop_body)
      // {
      //   error->type = UNE_ET_CONTINUE_OUTSIDE_LOOP;
      //   error->pos = tokens[*token_index].pos;
      //   error->__line__ = __LINE__;
      //   strcpy(error->__file__, __FILE__);
      //   #ifdef UNE_DEBUG_SOFT_ERROR
      //     CREATE_ERROR_NODE;
      //     (*token_index)++;
      //     return error_node;
      //   #else
      //     return NULL;
      //   #endif
      // }
      une_node *continuenode = une_node_create(UNE_NT_CONTINUE);
      continuenode->pos = tokens[*token_index].pos;
      (*token_index)++;
      return continuenode; }
    #pragma endregion Continue
    
    #pragma region Variable Definition
    case UNE_TT_ID: {
      size_t token_index_before = *token_index; // Needed to backstep in case this is not a variable definition.
      
      une_node *target = une_parse_id(tokens, token_index, error);
      if(target == NULL)
      {
        #ifdef UNE_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          target = error_node;
        #else
          return NULL;
        #endif
      }
      
      une_node *varset = une_node_create(UNE_NT_SET);
      varset->pos.start = target->pos.start;
      varset->content.branch.a = target;
      
      // Check if SET_IDX - If true, adjust the node type.
      if(tokens[*token_index].type == UNE_TT_LSQB)
      {
        (*token_index)++;
        
        une_node *position = une_parse_conditional_operation(tokens, token_index, error);
        if(position == NULL)
        {
          #ifdef UNE_DEBUG_SOFT_ERROR
            CREATE_ERROR_NODE;
            position = error_node;
          #else
            une_node_free(varset);
            return NULL;
          #endif
        }
        
        if(tokens[*token_index].type != UNE_TT_RSQB)
        {
          #ifdef UNE_DEBUG_SOFT_ERROR
            CREATE_ERROR_NODE;
            return error_node;
          #else
            une_node_free(varset);
            une_node_free(position);
            return NULL;
          #endif
        }
        (*token_index)++;
        
        varset->type = UNE_NT_SET_IDX;
        varset->content.branch.b = position;
      }
      
      if(tokens[*token_index].type == UNE_TT_SET)
      {
        (*token_index)++;
        
        une_node *conditional_operation = une_parse_conditional_operation(tokens, token_index, error);
        if(conditional_operation == NULL)
        {
          #ifdef UNE_DEBUG_SOFT_ERROR
            CREATE_ERROR_NODE;
            conditional_operation = error_node;
          #else
            une_node_free(varset);
            return NULL;
          #endif
        }
        
        varset->pos.end = conditional_operation->pos.end;
        if(varset->type == UNE_NT_SET_IDX)
        {
          varset->content.branch.c = conditional_operation;
        }
        else
        {
          varset->content.branch.b = conditional_operation;
        }
        return varset;
      }
      else
      {
        une_node_free(varset);
        *token_index = token_index_before;
        /*
        Notice how there is no break here: If the above code ended here it means
        the stmt is not a variable definition, leaving the only option
        for it to be a conditional operation.
        */
      } }
    #pragma endregion Variable Definition

    #pragma region Conditional Operation
    default: {
      une_node *expression = une_parse_conditional_operation(tokens, token_index, error);
      if(expression == NULL)
      {
        #ifdef UNE_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          return error_node;
        #else
          return NULL;
        #endif
      }
      return expression; }
    #pragma endregion Conditional Operation
  }
}
#pragma endregion une_parse_stmt

#pragma region une_parse_if
une_node *une_parse_if(
  une_token *tokens, size_t *token_index, une_error *error,
  une_token_type start_token)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // if or elif
  if(tokens[*token_index].type != start_token)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->values[0]._int = (int)start_token;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;

  // condition
  une_node *condition = une_parse_conditional_operation(tokens, token_index, error);
  if(condition == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      condition = error_node;
    #else
      return NULL;
    #endif
  }
  
  // stmt
  une_node *truebody = une_parse_stmt(tokens, token_index, error);
  if(truebody == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      truebody = error_node;
    #else
      une_node_free(condition);
      return NULL;
    #endif
  }

  une_node *ifstmt = une_node_create(UNE_NT_IF);
  ifstmt->pos.start = condition->pos.start; // Is overwritten later.
  ifstmt->pos.end = truebody->pos.end;
  ifstmt->content.branch.a = condition;
  ifstmt->content.branch.b = truebody;

  /*
  Whitespace. This counts for both 'elif' and 'else' because 'elif'
  creates an entirely new if node where this stmt then removes
  whitespace in front of 'else'.
  */
  if(tokens[*token_index].type == UNE_TT_NEW) (*token_index)++;

  if(tokens[*token_index].type == UNE_TT_ELIF)
  {
    // elif ...
    une_node *falsebody = une_parse_if(tokens, token_index, error, UNE_TT_ELIF);
    if(falsebody == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        falsebody = error_node;
      #else
        une_node_free(ifstmt);
        return NULL;
      #endif
    }
    ifstmt->content.branch.c = falsebody;
  }
  else if(tokens[*token_index].type == UNE_TT_ELSE)
  {
    // else stmt
    (*token_index)++;
    une_node *falsebody = une_parse_stmt(tokens, token_index, error);
    if(falsebody == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        falsebody = error_node;
      #else
        une_node_free(ifstmt);
        return NULL;
      #endif
    }
    ifstmt->content.branch.c = falsebody;
  }

  return ifstmt;
}
#pragma endregion une_parse_if

#pragma region Parse For Loop
une_node *une_parse_for(une_token *tokens, size_t *token_index, une_error *error)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // for
  if(tokens[*token_index].type != UNE_TT_FOR)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->values[0]._int = UNE_TT_FOR;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;
  
  // i
  une_node *counter = une_parse_id(tokens, token_index, error);
  if(counter == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      counter = error_node;
    #else
      return NULL;
    #endif
  }
  
  // from
  if(tokens[*token_index].type != UNE_TT_FROM)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->values[0]._int = UNE_TT_FROM;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      une_node_free(counter);
      return NULL;
    #endif
  }
  (*token_index)++;
  
  // 1
  une_node *from = une_parse_conditional_operation(tokens, token_index, error);
  if(from == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      from = error_node;
    #else
      une_node_free(counter);
      une_node_free(from);
      return NULL;
    #endif
  }
  
  // to
  if(tokens[*token_index].type != UNE_TT_TO)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->values[0]._int = UNE_TT_TO;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;
  
  // 10
  une_node *to = une_parse_conditional_operation(tokens, token_index, error);
  if(to == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      to = error_node;
    #else
      une_node_free(counter);
      une_node_free(from);
      return NULL;
    #endif
  }
  
  // ({) ... (})
  une_node *body = une_parse_stmt(tokens, token_index, error);
  if(body == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      body = error_node;
    #else
      une_node_free(counter);
      une_node_free(from);
      une_node_free(to);
      return NULL;
    #endif
  }
  
  // i, 1, 10, ...
  une_node *node = une_node_create(UNE_NT_FOR);
  node->pos = (une_position){pos_start, tokens[(*token_index)-1].pos.end};
  node->content.branch.a = counter;
  node->content.branch.b = from;
  node->content.branch.c = to;
  node->content.branch.d = body;
  return node;
}
#pragma endregion une_parse_for

#pragma region une_parse_while
une_node *une_parse_while(une_token *tokens, size_t *token_index, une_error *error)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // while
  if(tokens[*token_index].type != UNE_TT_WHILE)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->values[0]._int = UNE_TT_WHILE;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;
  
  // condition
  une_node *condition = une_parse_conditional_operation(tokens, token_index, error);
  if(condition == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      condition = error_node;
    #else
      return NULL;
    #endif
  }
  
  // { ... }
  une_node *body = une_parse_stmt(tokens, token_index, error);
  if(body == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      body = error_node;
    #else
      une_node_free(condition);
      return NULL;
    #endif
  }
  
  une_node *node = une_node_create(UNE_NT_WHILE);
  node->pos = (une_position){pos_start, body->pos.end};
  node->content.branch.a = condition;
  node->content.branch.b = body;
  return node;
}
#pragma endregion une_parse_while

#pragma region une_parse_def
une_node *une_parse_def(une_token *tokens, size_t *token_index, une_error *error)
{
  size_t pos_start = tokens[*token_index].pos.start;

  // def
  if(tokens[*token_index].type != UNE_TT_DEF)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)UNE_TT_DEF;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;

  // name
  une_node *name = une_parse_id(tokens, token_index, error);
  if(name == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      name = error_node;
    #else
      return NULL;
    #endif
  }

  // (
  if(tokens[*token_index].type != UNE_TT_LPAR)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)UNE_TT_LPAR;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      une_node_free(name);
      return NULL;
    #endif
  }

  une_node *params = une_node_create(UNE_NT_LIST);
  params->pos.start = tokens[*token_index].pos.start;

  (*token_index)++;

  // parameters
  une_node ** sequence = une_parse_sequence(
    tokens, token_index, error,
    &une_parse_id, UNE_TT_SEP, UNE_TT_RPAR
  );
  if(sequence == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_LIST;
      sequence = error_list;
    #else
      une_node_free(name);
      une_node_free(params);
      return NULL;
    #endif
  }

  params->pos.end = tokens[*token_index].pos.end;

  // ) // FIXME: Add redundant check here?
  (*token_index)++;

  params->content.value._vpp = (void**)sequence;

  // { ... }
  une_node *body = une_parse_stmt(tokens, token_index, error);
  if(body == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      body = error_node;
    #else
      une_node_free(name);
      une_node_free(params);
      return NULL;
    #endif
  }

  une_node *fndef = une_node_create(UNE_NT_DEF);
  fndef->pos = (une_position){pos_start, body->pos.end};
  fndef->content.branch.a = name;
  fndef->content.branch.b = params;
  fndef->content.branch.c = body;

  return fndef;
}
#pragma endregion une_parse_def

#pragma region une_parse_conditional_operation
une_node *une_parse_conditional_operation(une_token *tokens, size_t *token_index, une_error *error)
{
  une_node *trueconditions = une_parse_conditions(tokens, token_index, error);
  if(trueconditions == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      trueconditions = error_node;
    #else
      return NULL;
    #endif
  }
  
  if(tokens[*token_index].type == UNE_TT_IF)
  {
    (*token_index)++;
    
    une_node *condition = une_parse_conditions(tokens, token_index, error);
    if(condition == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        condition = error_node;
      #else
        une_node_free(trueconditions);
        return NULL;
      #endif
    }
    
    if(tokens[*token_index].type != UNE_TT_ELSE)
    {
      error->type = UNE_ET_EXPECTED_TOKEN;
      error->values[0]._int = (int)UNE_TT_ELSE;
      error->pos = tokens[*token_index].pos;
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        return error_node;
      #else
        une_node_free(trueconditions);
        une_node_free(condition);
        return NULL;
      #endif
    }
    (*token_index)++;
    
    une_node *falseconditions = une_parse_conditional_operation(tokens, token_index, error);
    if(falseconditions == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        falseconditions = error_node;
      #else
        une_node_free(trueconditions);
        une_node_free(condition);
        return NULL;
      #endif
    }
    
    une_node *conditional_operation = une_node_create(UNE_NT_COP);
    conditional_operation->pos.start = trueconditions->pos.start;
    conditional_operation->pos.end = falseconditions->pos.end;
    conditional_operation->content.branch.a = trueconditions;
    conditional_operation->content.branch.b = condition;
    conditional_operation->content.branch.c = falseconditions;
    trueconditions = conditional_operation;
  }
  
  return trueconditions;
}
#pragma endregion une_parse_conditional_operation

#pragma region une_parse_conditions
une_node *une_parse_conditions(une_token *tokens, size_t *token_index, une_error *error)
{
  une_node *left = une_parse_condition(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == UNE_TT_AND
     || tokens[*token_index].type == UNE_TT_OR)
  {
    une_node *new_left = une_node_create(
      tokens[*token_index].type == UNE_TT_AND ? UNE_NT_AND : UNE_NT_OR);
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    une_node *right = une_parse_condition(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        une_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion une_parse_conditions

#pragma region une_parse_condition
une_node *une_parse_condition(une_token *tokens, size_t *token_index, une_error *error)
{
  if(tokens[*token_index].type == UNE_TT_NOT)
  {
    une_node *not = une_node_create(UNE_NT_NOT);
    not->pos.start = tokens[*token_index].pos.start;
    (*token_index)++;
    
    une_node *condition = une_parse_condition(tokens, token_index, error);
    if(condition == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        condition = error_node;
      #else
        une_node_free(not);
        return NULL;
      #endif
    }
    
    not->pos.end = condition->pos.end;
    not->content.branch.a = condition;
    
    return not;
  }
  
  une_node *left = une_parse_expression(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == UNE_TT_EQU
     || tokens[*token_index].type == UNE_TT_NEQ
     || tokens[*token_index].type == UNE_TT_GTR
     || tokens[*token_index].type == UNE_TT_GEQ
     || tokens[*token_index].type == UNE_TT_LSS
     || tokens[*token_index].type == UNE_TT_LEQ)
  {
    une_node *new_left = une_node_create(UNE_NT_EQU);
    switch(tokens[*token_index].type)
    {
      case UNE_TT_EQU: new_left->type = UNE_NT_EQU; break;
      case UNE_TT_NEQ: new_left->type = UNE_NT_NEQ; break;
      case UNE_TT_GTR: new_left->type = UNE_NT_GTR; break;
      case UNE_TT_GEQ: new_left->type = UNE_NT_GEQ; break;
      case UNE_TT_LSS: new_left->type = UNE_NT_LSS; break;
      case UNE_TT_LEQ: new_left->type = UNE_NT_LEQ; break;
      default: WERR(L"what?");
    }
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    une_node *right = une_parse_expression(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        une_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion une_parse_condition

#pragma region une_parse_expression
une_node *une_parse_expression(une_token *tokens, size_t *token_index, une_error *error)
{
  une_node *left = une_parse_term(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == UNE_TT_ADD
     || tokens[*token_index].type == UNE_TT_SUB)
  {
    une_node *new_left = une_node_create(
      tokens[*token_index].type == UNE_TT_ADD ? UNE_NT_ADD : UNE_NT_SUB);
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    une_node *right = une_parse_term(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        une_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion une_parse_expression

#pragma region une_parse_term
une_node *une_parse_term(une_token *tokens, size_t *token_index, une_error *error)
{
  une_node *left = une_parse_power(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == UNE_TT_MUL
     || tokens[*token_index].type == UNE_TT_DIV
     || tokens[*token_index].type == UNE_TT_FDIV
     || tokens[*token_index].type == UNE_TT_MOD)
  {
    une_node *new_left = une_node_create(UNE_NT_MUL);
    switch(tokens[*token_index].type)
    {
      case UNE_TT_MUL: new_left->type = UNE_NT_MUL; break;
      case UNE_TT_DIV: new_left->type = UNE_NT_DIV; break;
      case UNE_TT_FDIV: new_left->type = UNE_NT_FDIV; break;
      case UNE_TT_MOD: new_left->type = UNE_NT_MOD; break;
      default: WERR(L"what?!"); // FIXME: Ugly
    }
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    une_node *right = une_parse_power(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        une_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion une_parse_term

#pragma region une_parse_power
une_node *une_parse_power(une_token *tokens, size_t *token_index, une_error *error)
{
  une_node *left = une_parse_index(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == UNE_TT_POW)
  {
    une_node *new_left = une_node_create(UNE_NT_POW);
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    // FIXME: RECURSIVE WHEN IT DOESNT HAVE TO BE, SEE EXPRESSION
    une_node *right = une_parse_power(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        une_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion une_parse_power

#pragma region une_parse_index
une_node *une_parse_index(une_token *tokens, size_t *token_index, une_error *error)
{
  une_node *left = une_parse_atom(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == UNE_TT_LSQB)
  {
    une_node *new_left = une_node_create(UNE_NT_IDX);
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    une_node *right = une_parse_conditions(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        une_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    if(tokens[*token_index].type != UNE_TT_RSQB)
    {
      error->type = UNE_ET_EXPECTED_TOKEN;
      error->pos = tokens[*token_index].pos;
      error->values[0]._int = (int)UNE_TT_RSQB;
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        return error_node;
      #else
        une_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->pos.end = tokens[*token_index].pos.end;
    (*token_index)++;

    left = new_left;
  }
  
  return left;
}
#pragma endregion une_parse_index

#pragma region une_parse_atom
une_node *une_parse_atom(une_token *tokens, size_t *token_index, une_error *error)
{
  switch(tokens[*token_index].type)
  {
    case UNE_TT_SUB: {
      une_node *neg = une_node_create(UNE_NT_NEG);
      neg->pos.start = tokens[*token_index].pos.start;
      (*token_index)++;
      une_node *expression = une_parse_power(tokens, token_index, error);
      if(expression == NULL)
      {
        #ifdef UNE_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          expression = error_node;
        #else
          une_node_free(neg);
          return NULL;
        #endif
      }
      neg->content.branch.a = expression;
      neg->pos.end = expression->pos.end;
      return neg; }

    case UNE_TT_INT: {
      une_node *num = une_node_create(UNE_NT_INT);
      num->pos = tokens[*token_index].pos;
      num->content.value._int = tokens[*token_index].value._int;
      (*token_index)++;
      return num; }
    
    case UNE_TT_FLT: {
      une_node *num = une_node_create(UNE_NT_FLT);
      num->pos = tokens[*token_index].pos;
      num->content.value._flt = tokens[*token_index].value._flt;
      (*token_index)++;
      return num; }
    
    case UNE_TT_STR: {
      une_node *str = une_node_create(UNE_NT_STR);
      str->pos = tokens[*token_index].pos;
      // DOC: Memory Management: This shows that nodes reference tokens' WCS instead of storing their own.
      str->content.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;
      return str; }
    
    case UNE_TT_ID: {
      une_node *node = une_node_create(UNE_NT_ID);
      node->pos = tokens[*token_index].pos;
      node->content.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;

      // Function Call
      if(tokens[*token_index].type == UNE_TT_LPAR)
      {
        une_node *fncall = une_node_create(UNE_NT_CALL);
        fncall->pos.start = node->pos.start;
        fncall->content.branch.a = node;

        une_node *params = une_node_create(UNE_NT_LIST);
        params->pos.start = tokens[*token_index].pos.start;
        (*token_index)++;
        
        une_node ** sequence = une_parse_sequence(
          tokens, token_index, error,
          &une_parse_conditions, UNE_TT_SEP, UNE_TT_RPAR
        );
        if(sequence == NULL)
        {
          #ifdef UNE_DEBUG_SOFT_ERROR
            CREATE_ERROR_LIST;
            sequence = error_list;
          #else
            une_node_free(fncall);
            une_node_free(params);
            return NULL;
          #endif
        }
        params->content.value._vpp = (void**)sequence;
        params->pos.end = tokens[*token_index].pos.end;
        (*token_index)++;

        fncall->pos.end = params->pos.end;
        fncall->content.branch.b = params;
        node = fncall;
      }
      
      // Get Variable
      else
      {
        une_node *varget = une_node_create(UNE_NT_GET);
        varget->pos = node->pos;
        varget->content.branch.a = node;
        node = varget;
      }
      
      return node; }
    
    case UNE_TT_LSQB: {
      une_node *list = une_node_create(UNE_NT_LIST);
      list->pos.start = tokens[*token_index].pos.start;
      // [
      (*token_index)++;
      // ...
      une_node ** sequence = une_parse_sequence(
        tokens, token_index, error,
        &une_parse_conditions, UNE_TT_SEP, UNE_TT_RSQB
      );
      if(sequence == NULL)
      {
        #ifdef UNE_DEBUG_SOFT_ERROR
          CREATE_ERROR_LIST;
          sequence = error_list;
        #else
          une_node_free(list);
          return NULL;
        #endif
      }
      // ] // FIXME: Add redundant check here?
      list->pos.end = tokens[*token_index].pos.end;
      (*token_index)++;
      list->content.value._vpp = (void**)sequence;
      return list; }
    
    case UNE_TT_LPAR: {
      (*token_index)++;
      une_node *expression = une_parse_conditional_operation(tokens, token_index, error);
      if(expression == NULL)
      {
        #ifdef UNE_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          expression = error_node;
        #else
          return NULL;
        #endif
      }
      if(tokens[*token_index].type != UNE_TT_RPAR)
      {
        une_node_free(expression);
        error->type = UNE_ET_EXPECTED_TOKEN;
        error->pos = tokens[*token_index].pos;
        error->values[0]._int = (int)UNE_TT_RPAR;
        error->__line__ = __LINE__;
        strcpy(error->__file__, __FILE__);
        #ifdef UNE_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          (*token_index)++;
          return error_node;
        #else
          return NULL;
        #endif
      }
      (*token_index)++;
      return expression; }
    
    default: {
      error->type = UNE_ET_UNEXPECTED_TOKEN;
      error->pos = tokens[*token_index].pos;
      error->values[0]._int = (int)(tokens[*token_index].type);
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef UNE_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        return error_node;
      #else
        return NULL;
      #endif
      }
  }
}
#pragma endregion une_parse_atom

#pragma region une_parse_id
une_node *une_parse_id(une_token *tokens, size_t *token_index, une_error *error)
{
  if(tokens[*token_index].type != UNE_TT_ID)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->values[0]._int = UNE_TT_ID;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  une_node *id = une_node_create(UNE_NT_ID);
  id->pos = tokens[*token_index].pos;
  id->content.value._wcs = tokens[*token_index].value._wcs;
  (*token_index)++;
  return id;
}
#pragma endregion une_parse_id

#pragma region une_parse_return
une_node *une_parse_return(une_token *tokens, size_t *token_index, une_error *error)
{
  une_node *returnnode = une_node_create(UNE_NT_RETURN);
  returnnode->pos = tokens[*token_index].pos; /* If the parser finds a return value after this,
                                                 pos.end is changed further down */

  // return
  if(tokens[*token_index].type != UNE_TT_RETURN)
  {
    error->type = UNE_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)UNE_TT_RETURN;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef UNE_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;

  // value?
  une_node *value;
  if(tokens[*token_index].type != UNE_TT_NEW
  && tokens[*token_index].type != UNE_TT_EOF) // FIXME: ?
  {
    value = une_parse_conditional_operation(tokens, token_index, error);
    // DOC: NULL _here_ means an error.
    if(value == NULL) WERR(L"Out of memory.");
    returnnode->pos.end = value->pos.end;
  }
  else
  {
    // DOC: NULL _here_ means no return value was specified. This tells the interpreter
    // not to try to interpret a return value where there is none.
    value = NULL;
  }
  
  returnnode->content.branch.a = value;
  
  return returnnode;
}
#pragma endregion une_parse_return