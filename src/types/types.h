/*
types.h - Une
Modified 2023-12-10
*/

#ifndef UNE_TYPES_H
#define UNE_TYPES_H

/* Header-specific includes. */
#include <stdio.h>
#include "../struct/result.h"
#include "../struct/error.h"
#include "../struct/interpreter_state.h"
#include "void.h"
#include "int.h"
#include "flt.h"
#include "str.h"
#include "list.h"
#include "object.h"
#include "function.h"
#include "native.h"
#include "../struct/engine.h"

/*
Holds all the functionality a type can define.
*/
typedef struct une_type_ {
	une_result_kind kind;
	
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
	
	bool (*is_valid_index)(une_result, une_result);
	une_result (*refer_to_index)(une_result, une_result);
	bool (*is_valid_range)(une_result, une_result, une_result);
	une_result (*refer_to_range)(une_result, une_result, une_result);
	
	bool (*member_exists)(une_result, wchar_t*);
	une_result (*refer_to_member)(une_result, wchar_t*);
	
	bool (*can_assign)(une_reference, une_result);
	void (*assign)(une_reference, une_result);
	
	une_result (*copy)(une_result);
	void (*free_members)(une_result);
	
	une_result (*call)(une_node*, une_result, une_result, wchar_t*);
} une_type;

extern une_type une_types[];

/*
*** Interface.
*/

une_type UNE_TYPE_FOR_RESULT_KIND(une_result_kind kind);
une_type UNE_TYPE_FOR_RESULT(une_result result);

#endif /* UNE_TYPES_H */
