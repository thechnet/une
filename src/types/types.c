/*
types.c - Une
Modified 2023-12-10
*/

/* Header-specific includes. */
#include "types.h"

/* Implementation-specific includes. */
#include "int.h"
#include "flt.h"
#include "str.h"
#include "list.h"
#include "void.h"
#include "function.h"
#include "native.h"

une_type une_types[] = {
	{
		.kind = UNE_RK_VOID,
		.represent = &une_type_void_represent,
		.is_true = &une_type_void_is_true,
		.is_equal = &une_type_void_is_equal,
	},
	{
		.kind = UNE_RK_INT,
		.as_int = &une_type_int_as_int,
		.as_flt = &une_type_int_as_flt,
		.as_str = &une_type_int_as_str,
		.represent = &une_type_int_represent,
		.is_true = &une_type_int_is_true,
		.is_equal = &une_type_int_is_equal,
		.is_greater = &une_type_int_is_greater,
		.is_greater_or_equal = &une_type_int_is_greater_or_equal,
		.is_less = &une_type_int_is_less,
		.is_less_or_equal = &une_type_int_is_less_or_equal,
		.add = &une_type_int_add,
		.sub = &une_type_int_sub,
		.mul = &une_type_int_mul,
		.div = &une_type_int_div,
		.fdiv = &une_type_int_fdiv,
		.mod = &une_type_int_mod,
		.pow = &une_type_int_pow,
		.negate = &une_type_int_negate,
	},
	{
		.kind = UNE_RK_FLT,
		.as_int = &une_type_flt_as_int,
		.as_flt = &une_type_flt_as_flt,
		.as_str = &une_type_flt_as_str,
		.represent = &une_type_flt_represent,
		.is_true = &une_type_flt_is_true,
		.is_equal = &une_type_flt_is_equal,
		.is_greater = &une_type_flt_is_greater,
		.is_greater_or_equal = &une_type_flt_is_greater_or_equal,
		.is_less = &une_type_flt_is_less,
		.is_less_or_equal = &une_type_flt_is_less_or_equal,
		.add = &une_type_flt_add,
		.sub = &une_type_flt_sub,
		.mul = &une_type_flt_mul,
		.div = &une_type_flt_div,
		.fdiv = &une_type_flt_fdiv,
		.mod = &une_type_flt_mod,
		.pow = &une_type_flt_pow,
		.negate = &une_type_flt_negate,
	},
	{
		.kind = UNE_RK_STR,
		.as_int = &une_type_str_as_int,
		.as_flt = &une_type_str_as_flt,
		.as_str = &une_type_str_as_str,
		.represent = &une_type_str_represent,
		.is_true = &une_type_str_is_true,
		.is_equal = &une_type_str_is_equal,
		.is_greater = &une_type_str_is_greater,
		.is_greater_or_equal = &une_type_str_is_greater_or_equal,
		.is_less = &une_type_str_is_less,
		.is_less_or_equal = &une_type_str_is_less_or_equal,
		.add = &une_type_str_add,
		.mul = &une_type_str_mul,
		.get_len = &une_type_str_get_len,
		.is_valid_index = &une_type_str_is_valid_index,
		.refer_to_index = &une_type_str_refer_to_index,
		.is_valid_range = &une_type_str_is_valid_range,
		.refer_to_range = &une_type_str_refer_to_range,
		.can_assign = &une_type_str_can_assign,
		.assign = &une_type_str_assign,
		.copy = &une_type_str_copy,
		.free_members = &une_type_str_free_members,
	},
	{
		.kind = UNE_RK_LIST,
		.represent = &une_type_list_represent,
		.is_true = &une_type_list_is_true,
		.is_equal = &une_type_list_is_equal,
		.is_greater = &une_type_list_is_greater,
		.is_greater_or_equal = &une_type_list_is_greater_or_equal,
		.is_less = &une_type_list_is_less,
		.is_less_or_equal = &une_type_list_is_less_or_equal,
		.add = &une_type_list_add,
		.mul = &une_type_list_mul,
		.get_len = &une_type_list_get_len,
		.is_valid_index = &une_type_list_is_valid_index,
		.refer_to_index = &une_type_list_refer_to_index,
		.is_valid_range = &une_type_list_is_valid_range,
		.refer_to_range = &une_type_list_refer_to_range,
		.can_assign = &une_type_list_can_assign,
		.assign = &une_type_list_assign,
		.copy = &une_type_list_copy,
		.free_members = &une_type_list_free_members,
	},
	{
		.kind = UNE_RK_OBJECT,
		.represent = &une_type_object_represent,
		.is_true = &une_type_object_is_true,
		.is_equal = &une_type_object_is_equal,
		.member_exists = &une_type_object_member_exists,
		.refer_to_member = &une_type_object_refer_to_member,
		.copy = &une_type_object_copy,
		.free_members = &une_type_object_free_members,
	},
	{
		.kind = UNE_RK_FUNCTION,
		.represent = &une_type_function_represent,
		.is_true = &une_type_function_is_true,
		.is_equal = &une_type_function_is_equal,
		.call = &une_type_function_call,
	},
	{
		.kind = UNE_RK_NATIVE,
		.represent = &une_type_native_represent,
		.is_true = &une_type_native_is_true,
		.is_equal = &une_type_native_is_equal,
		.call = &une_type_native_call,
	},
};

/*
*** Interface.
*/

une_type UNE_TYPE_FOR_RESULT_KIND(une_result_kind kind)
{
	assert(UNE_RESULT_KIND_IS_TYPE(kind));
	return une_types[kind-UNE_R_BGN_DATA_RESULT_KINDS];
}

une_type UNE_TYPE_FOR_RESULT(une_result result)
{
	if (result.kind == UNE_RK_REFERENCE) {
		if (result.reference.kind == UNE_FK_LISTVIEW)
			return UNE_TYPE_FOR_RESULT_KIND(UNE_RK_LIST);
		if (result.reference.kind == UNE_FK_STRVIEW)
			return UNE_TYPE_FOR_RESULT_KIND(UNE_RK_STR);
		return UNE_TYPE_FOR_RESULT(*(une_result*)result.reference.root);
	}
	return UNE_TYPE_FOR_RESULT_KIND(result.kind);
}
