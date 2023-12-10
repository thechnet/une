/*
int.c - Une
Modified 2023-12-10
*/

/* Header-specific includes. */
#include "int.h"

/* Implementation-specific includes. */
#include <math.h>
#include "../tools.h"
#include "str.h"
#include "list.h"

/*
Convert to INT.
*/
une_result une_type_int_as_int(une_result result)
{
	assert(result.kind == UNE_RK_INT);
	return result;
}

/*
Convert to FLT.
*/
une_result une_type_int_as_flt(une_result result)
{
	assert(result.kind == UNE_RK_INT);
	return (une_result){
		.kind = UNE_RK_FLT,
		.value._flt = (une_flt)result.value._int
	};
}

/*
Convert to STR.
*/
une_result une_type_int_as_str(une_result result)
{
	assert(result.kind == UNE_RK_INT);
	wchar_t *out = malloc(UNE_SIZE_NUMBER_AS_STRING*sizeof(*out));
	verify(out);
	swprintf(out, UNE_SIZE_NUMBER_AS_STRING, UNE_PRINTF_UNE_INT, result.value._int);
	return (une_result){
		.kind = UNE_RK_STR,
		.value._wcs = out
	};
}

/*
Print a text representation to file.
*/
void une_type_int_represent(FILE *file, une_result result)
{
	assert(result.kind == UNE_RK_INT);
	fwprintf(file, UNE_PRINTF_UNE_INT, result.value._int);
}

/*
Check for truth.
*/
une_int une_type_int_is_true(une_result result)
{
	assert(result.kind == UNE_RK_INT);
	return result.value._int == 0 ? 0 : 1;
}

/*
Check if subject is equal to comparison.
*/
une_int une_type_int_is_equal(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_INT);
	if (comparison.kind == UNE_RK_INT)
		return subject.value._int == comparison.value._int;
	if (comparison.kind == UNE_RK_FLT)
		return une_flts_equal((une_flt)subject.value._int, comparison.value._flt);
	return 0;
}

/*
Check if subject is greater than comparison.
*/
une_int une_type_int_is_greater(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_INT);
	if (comparison.kind == UNE_RK_INT)
		return subject.value._int > comparison.value._int;
	if (comparison.kind == UNE_RK_FLT)
		return (une_flt)subject.value._int > comparison.value._flt;
	return -1;
}

/*
Check if subject is greater than or equal to comparison.
*/
une_int une_type_int_is_greater_or_equal(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_INT);
	if (comparison.kind == UNE_RK_INT)
		return subject.value._int >= comparison.value._int;
	if (comparison.kind == UNE_RK_FLT)
		return (une_flt)subject.value._int >= comparison.value._flt;
	return -1;
}

/*
Check is subject is less than comparison.
*/
une_int une_type_int_is_less(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_INT);
	if (comparison.kind == UNE_RK_INT)
		return subject.value._int < comparison.value._int;
	if (comparison.kind == UNE_RK_FLT)
		return (une_flt)subject.value._int < comparison.value._flt;
	return -1;
}

/*
Check if subject is less than or equal to comparison.
*/
une_int une_type_int_is_less_or_equal(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_INT);
	if (comparison.kind == UNE_RK_INT)
		return subject.value._int <= comparison.value._int;
	if (comparison.kind == UNE_RK_FLT)
		return (une_flt)subject.value._int <= comparison.value._flt;
	return -1;
}

/*
Add right to left.
*/
une_result une_type_int_add(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_INT);
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_INT,
			.value._int = left.value._int + right.value._int
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = (une_flt)left.value._int + right.value._flt
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Subtract right from left.
*/
une_result une_type_int_sub(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_INT);
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_INT,
			.value._int = left.value._int - right.value._int
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = (une_flt)left.value._int - right.value._flt
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Multiply left by right.
*/
une_result une_type_int_mul(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_INT);
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_INT,
			.value._int = left.value._int * right.value._int
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = (une_flt)left.value._int * right.value._flt
		};
	if (right.kind == UNE_RK_STR)
		return une_type_str_mul(right, left);
	if (right.kind == UNE_RK_LIST)
		return une_type_list_mul(right, left);
	return une_result_create(UNE_RK_ERROR);
}

/*
Divide left by right.
*/
une_result une_type_int_div(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_INT);
	
	/* Returns INFINITY on zero division. */
	if (right.kind == UNE_RK_INT) {
		if (right.value._int == 0)
			return (une_result){
				.kind = UNE_RK_FLT,
				.value._flt = UNE_INFINITY
			};
		if (left.value._int % right.value._int == 0)
			return (une_result){
				.kind = UNE_RK_INT,
				.value._int = left.value._int / right.value._int
			};
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = (une_flt)left.value._int / (une_flt)right.value._int
		};
	}
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = (une_flt)left.value._int / right.value._flt
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Floor divide left by right.
*/
une_result une_type_int_fdiv(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_INT);
	
	/* Return INFINITY on zero division. */
	if ((right.kind == UNE_RK_INT && right.value._int == 0) || (right.kind == UNE_RK_FLT && une_flts_equal(right.value._flt, 0.0L)))
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = UNE_INFINITY
		};
	
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_INT,
			.value._int = left.value._int / right.value._int
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_INT,
			.value._int = (une_int)(floorl((une_flt)left.value._int / right.value._flt))
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Get the modulus of left and right.
*/
une_result une_type_int_mod(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_INT);
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_INT,
			.value._int = left.value._int % right.value._int
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = fmodl((une_flt)left.value._int, right.value._flt)
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Raise left to the power of right.
*/
une_result une_type_int_pow(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_INT);
	
	/* Returns NaN on unreal number. */
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_INT,
			.value._int = (une_int)(powl((une_flt)left.value._int, (une_flt)right.value._int))
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = powl((une_flt)left.value._int, right.value._flt)
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Negate the result.
*/
une_result une_type_int_negate(une_result result)
{
	assert(result.kind == UNE_RK_INT);
	return (une_result){
		.kind = UNE_RK_INT,
		.value._int = -result.value._int
	};
}
