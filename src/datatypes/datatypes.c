/*
datatypes.c - Une
Modified 2023-05-05
*/

/* Header-specific includes. */
#include "datatypes.h"

/* Implementation-specific includes. */
#include "int.h"
#include "flt.h"
#include "str.h"
#include "list.h"
#include "void.h"
#include "function.h"
#include "builtin.h"

une_datatype une_datatypes[] = {
  {
    .type = UNE_RT_VOID,
    .represent = &une_datatype_void_represent,
    .is_true = &une_datatype_void_is_true,
    .is_equal = &une_datatype_void_is_equal,
  },
  {
    .type = UNE_RT_INT,
    .as_int = &une_datatype_int_as_int,
    .as_flt = &une_datatype_int_as_flt,
    .as_str = &une_datatype_int_as_str,
    .represent = &une_datatype_int_represent,
    .is_true = &une_datatype_int_is_true,
    .is_equal = &une_datatype_int_is_equal,
    .is_greater = &une_datatype_int_is_greater,
    .is_greater_or_equal = &une_datatype_int_is_greater_or_equal,
    .is_less = &une_datatype_int_is_less,
    .is_less_or_equal = &une_datatype_int_is_less_or_equal,
    .add = &une_datatype_int_add,
    .sub = &une_datatype_int_sub,
    .mul = &une_datatype_int_mul,
    .div = &une_datatype_int_div,
    .fdiv = &une_datatype_int_fdiv,
    .mod = &une_datatype_int_mod,
    .pow = &une_datatype_int_pow,
    .negate = &une_datatype_int_negate,
  },
  {
    .type = UNE_RT_FLT,
    .as_int = &une_datatype_flt_as_int,
    .as_flt = &une_datatype_flt_as_flt,
    .as_str = &une_datatype_flt_as_str,
    .represent = &une_datatype_flt_represent,
    .is_true = &une_datatype_flt_is_true,
    .is_equal = &une_datatype_flt_is_equal,
    .is_greater = &une_datatype_flt_is_greater,
    .is_greater_or_equal = &une_datatype_flt_is_greater_or_equal,
    .is_less = &une_datatype_flt_is_less,
    .is_less_or_equal = &une_datatype_flt_is_less_or_equal,
    .add = &une_datatype_flt_add,
    .sub = &une_datatype_flt_sub,
    .mul = &une_datatype_flt_mul,
    .div = &une_datatype_flt_div,
    .fdiv = &une_datatype_flt_fdiv,
    .mod = &une_datatype_flt_mod,
    .pow = &une_datatype_flt_pow,
    .negate = &une_datatype_flt_negate,
  },
  {
    .type = UNE_RT_STR,
    .as_int = &une_datatype_str_as_int,
    .as_flt = &une_datatype_str_as_flt,
    .as_str = &une_datatype_str_as_str,
    .represent = &une_datatype_str_represent,
    .is_true = &une_datatype_str_is_true,
    .is_equal = &une_datatype_str_is_equal,
    .is_greater = &une_datatype_str_is_greater,
    .is_greater_or_equal = &une_datatype_str_is_greater_or_equal,
    .is_less = &une_datatype_str_is_less,
    .is_less_or_equal = &une_datatype_str_is_less_or_equal,
    .add = &une_datatype_str_add,
    .mul = &une_datatype_str_mul,
    .get_len = &une_datatype_str_get_len,
    .is_valid_index = &une_datatype_str_is_valid_index,
    .refer_to_index = &une_datatype_str_refer_to_index,
    .is_valid_range = &une_datatype_str_is_valid_range,
    .refer_to_range = &une_datatype_str_refer_to_range,
    .can_assign = &une_datatype_str_can_assign,
    .assign = &une_datatype_str_assign,
    .copy = &une_datatype_str_copy,
    .free_members = &une_datatype_str_free_members,
  },
  {
    .type = UNE_RT_LIST,
    .represent = &une_datatype_list_represent,
    .is_true = &une_datatype_list_is_true,
    .is_equal = &une_datatype_list_is_equal,
    .is_greater = &une_datatype_list_is_greater,
    .is_greater_or_equal = &une_datatype_list_is_greater_or_equal,
    .is_less = &une_datatype_list_is_less,
    .is_less_or_equal = &une_datatype_list_is_less_or_equal,
    .add = &une_datatype_list_add,
    .mul = &une_datatype_list_mul,
    .get_len = &une_datatype_list_get_len,
    .is_valid_index = &une_datatype_list_is_valid_index,
    .refer_to_index = &une_datatype_list_refer_to_index,
    .is_valid_range = &une_datatype_list_is_valid_range,
    .refer_to_range = &une_datatype_list_refer_to_range,
    .can_assign = &une_datatype_list_can_assign,
    .assign = &une_datatype_list_assign,
    .copy = &une_datatype_list_copy,
    .free_members = &une_datatype_list_free_members,
  },
  {
    .type = UNE_RT_OBJECT,
    .represent = &une_datatype_object_represent,
    .is_true = &une_datatype_object_is_true,
    .is_equal = &une_datatype_object_is_equal,
    .member_exists = &une_datatype_object_member_exists,
    .add_member = &une_datatype_object_add_member,
    .refer_to_member = &une_datatype_object_refer_to_member,
    .copy = &une_datatype_object_copy,
    .free_members = &une_datatype_object_free_members,
  },
  {
    .type = UNE_RT_FUNCTION,
    .represent = &une_datatype_function_represent,
    .is_true = &une_datatype_function_is_true,
    .copy = &une_datatype_function_copy,
    .free_members = &une_datatype_function_free_members,
    .call = &une_datatype_function_call,
  },
  {
    .type = UNE_RT_BUILTIN,
    .represent = &une_datatype_builtin_represent,
    .is_true = &une_datatype_builtin_is_true,
    .call = &une_datatype_builtin_call,
  },
};

/*
*** Interface.
*/

une_datatype UNE_DATATYPE_FOR_RESULT_TYPE(une_result_type type)
{
  assert(UNE_RESULT_TYPE_IS_DATA_TYPE(type));
  return une_datatypes[type-UNE_R_BGN_DATA_RESULT_TYPES];
}

une_datatype UNE_DATATYPE_FOR_RESULT(une_result result)
{
  if (result.type == UNE_RT_REFERENCE) {
    if (result.reference.type == UNE_FT_LISTVIEW)
      return UNE_DATATYPE_FOR_RESULT_TYPE(UNE_RT_LIST);
    if (result.reference.type == UNE_FT_STRVIEW)
      return UNE_DATATYPE_FOR_RESULT_TYPE(UNE_RT_STR);
    return UNE_DATATYPE_FOR_RESULT(*(une_result*)result.reference.root);
  }
  return UNE_DATATYPE_FOR_RESULT_TYPE(result.type);
}
