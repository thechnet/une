/*
flt.c - Une
Modified 2025-07-26
*/

/* Header-specific includes. */
#include "flt.h"

/* Implementation-specific includes. */
#include <math.h>
#include "../tools.h"

/*
Convert to INT.
*/
une_result une_type_flt_as_int(une_result result)
{
	assert(result.kind == UNE_RK_FLT);
	return (une_result){
		.kind = UNE_RK_INT,
		.value._int = (une_int)result.value._flt
	};
}

/*
Convert to FLT.
*/
une_result une_type_flt_as_flt(une_result result)
{
	assert(result.kind == UNE_RK_FLT);
	return result;
}

/*
Convert to STR.
*/
une_result une_type_flt_as_str(une_result result)
{
	assert(result.kind == UNE_RK_FLT);
	return (une_result){
		.kind = UNE_RK_STR,
		.value._wcs = une_flt_to_wcs(result.value._flt)
	};
}

/*
Print a text representation to file.
*/
void une_type_flt_represent(FILE *file, une_result result)
{
	assert(result.kind == UNE_RK_FLT);
	wchar_t *flt_as_wcs = une_flt_to_wcs(result.value._flt);
	fwprintf(file, L"%ls", flt_as_wcs);
	free(flt_as_wcs);
}

/*
Check for truth.
*/
une_int une_type_flt_is_true(une_result result)
{
	assert(result.kind == UNE_RK_FLT);
	return une_flts_equal(result.value._flt, UNE_NEW_FLT(0.0)) ? 0 : 1;
}

/*
Check if subject is equal to comparison.
*/
une_int une_type_flt_is_equal(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_FLT);
	if (comparison.kind == UNE_RK_INT)
		return une_flts_equal(subject.value._flt, (une_flt)comparison.value._int);
	if (comparison.kind == UNE_RK_FLT)
		return une_flts_equal(subject.value._flt, comparison.value._flt);
	return 0;
}

/*
Check if subject is greater than comparison.
*/
une_int une_type_flt_is_greater(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_FLT);
	if (comparison.kind == UNE_RK_INT)
		return subject.value._flt > (une_flt)comparison.value._int;
	if (comparison.kind == UNE_RK_FLT)
		return subject.value._flt > comparison.value._flt;
	return -1;
}

/*
Check if subject is greater than or equal to comparison.
*/
une_int une_type_flt_is_greater_or_equal(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_FLT);
	if (comparison.kind == UNE_RK_INT)
		return subject.value._flt >= (une_flt)comparison.value._int;
	if (comparison.kind == UNE_RK_FLT)
		return subject.value._flt >= comparison.value._flt;
	return -1;
}

/*
Check is subject is less than comparison.
*/
une_int une_type_flt_is_less(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_FLT);
	if (comparison.kind == UNE_RK_INT)
		return subject.value._flt < (une_flt)comparison.value._int;
	if (comparison.kind == UNE_RK_FLT)
		return subject.value._flt < comparison.value._flt;
	return -1;
}

/*
Check if subject is less than or equal to comparison.
*/
une_int une_type_flt_is_less_or_equal(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_FLT);
	if (comparison.kind == UNE_RK_INT)
		return subject.value._flt <= (une_flt)comparison.value._int;
	if (comparison.kind == UNE_RK_FLT)
		return subject.value._flt <= comparison.value._flt;
	return -1;
}

/*
Add right to left.
*/
une_result une_type_flt_add(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_FLT);
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = left.value._flt + (une_flt)right.value._int
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = left.value._flt + right.value._flt
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Subtract right from left.
*/
une_result une_type_flt_sub(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_FLT);
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = left.value._flt - (une_flt)right.value._int
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = left.value._flt - right.value._flt
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Multiply left by right.
*/
une_result une_type_flt_mul(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_FLT);
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = left.value._flt * (une_flt)right.value._int
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = left.value._flt * right.value._flt
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Divide left by right.
*/
une_result une_type_flt_div(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_FLT);
	
	/* Returns INFINITY on zero division. */
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = left.value._flt / (une_flt)right.value._int
		};
	if (right.kind == UNE_RK_FLT) {
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = left.value._flt / right.value._flt
		};
	}
	return une_result_create(UNE_RK_ERROR);
}

/*
Floor divide left by right.
*/
une_result une_type_flt_fdiv(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_FLT);
	
	/* Return INFINITY on zero division. */
	if ((right.kind == UNE_RK_INT && right.value._int == 0) || (right.kind == UNE_RK_FLT && une_flts_equal(right.value._flt, UNE_NEW_FLT(0.0))))
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = UNE_INFINITY
		};
	
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_INT,
			.value._int = (une_int)(une_flt_floor(left.value._flt / (une_flt)right.value._int))
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_INT,
			.value._int = (une_int)(une_flt_floor(left.value._flt / right.value._flt))
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Get the modulus of left and right.
*/
une_result une_type_flt_mod(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_FLT);
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = une_flt_mod(left.value._flt, (une_flt)right.value._int)
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = une_flt_mod(left.value._flt, right.value._flt)
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Raise left to the power of right.
*/
une_result une_type_flt_pow(une_result left, une_result right)
{
	assert(left.kind == UNE_RK_FLT);
	
	/* Returns NaN on unreal number. */
	if (right.kind == UNE_RK_INT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = une_flt_pow(left.value._flt, (une_flt)right.value._int)
		};
	if (right.kind == UNE_RK_FLT)
		return (une_result){
			.kind = UNE_RK_FLT,
			.value._flt = une_flt_pow(left.value._flt, right.value._flt)
		};
	return une_result_create(UNE_RK_ERROR);
}

/*
Negate the result.
*/
une_result une_type_flt_negate(une_result result)
{
	assert(result.kind == UNE_RK_FLT);
	return (une_result){
		.kind = UNE_RK_FLT,
		.value._flt = -result.value._flt
	};
}
