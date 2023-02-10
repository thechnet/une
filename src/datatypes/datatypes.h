/*
datatypes.h - Une
Modified 2023-02-10
*/

#ifndef UNE_DATATYPES_H
#define UNE_DATATYPES_H

/* Header-specific includes. */
#include <stdio.h>
#include "../types/result.h"
#include "../types/error.h"
#include "../types/interpreter_state.h"
#include "int.h"
#include "flt.h"
#include "str.h"
#include "list.h"
#include "void.h"

/*
Holds all the functionality a datatype can define.
*/
typedef struct une_datatype_ {
  une_result_type type;
  
  une_result (*as_int)(une_result);
  une_result (*as_flt)(une_result);
  une_result (*as_str)(une_result);
  void (*represent)(FILE*, une_result);
  
  une_int (*is_true)(une_result);
  une_int (*is_equal)(une_result, une_result);
  une_int (*is_greater)(une_result, une_result);
  une_int (*is_greater_or_equal)(une_result, une_result);
  une_int (*is_less)(une_result, une_result);
  une_int (*is_less_or_equal)(une_result, une_result);
  
  une_result (*add)(une_result, une_result);
  une_result (*sub)(une_result, une_result);
  une_result (*mul)(une_result, une_result);
  une_result (*div)(une_result, une_result);
  une_result (*fdiv)(une_result, une_result);
  une_result (*mod)(une_result, une_result);
  une_result (*pow)(une_result, une_result);
  une_result (*negate)(une_result);
  
  size_t (*get_len)(une_result);
  
  bool (*is_valid_index_type)(une_result_type);
  bool (*is_valid_index)(une_result, une_result);
  bool (*is_valid_element)(une_result);
  une_result (*get_index)(une_result, une_result);
  une_result (*seek_index)(une_result*, une_result);
  
  une_result (*copy)(une_result);
  void (*free_members)(une_result);
  
  une_result (*call)(une_error*, une_interpreter_state*, une_node*, une_result, une_result);
} une_datatype;

extern une_datatype une_datatypes[];

/*
*** Interface.
*/

#define UNE_DATATYPE_FOR_RESULT(result) \
  (une_datatypes[(result).type-UNE_R_BGN_DATA_RESULT_TYPES])

#define UNE_DATATYPE_FOR_RESULT_TYPE(result_type) \
  (une_datatypes[(result_type)-UNE_R_BGN_DATA_RESULT_TYPES])

#endif /* UNE_DATATYPES_H */
