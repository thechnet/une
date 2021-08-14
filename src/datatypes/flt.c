/*
flt.c - Une
Modified 2021-08-14
*/

/* Header-specific includes. */
#include "flt.h"

/* Implementation-specific includes. */
#include <math.h>
#include "../tools.h"

/*
Convert to INT.
*/
une_result une_datatype_flt_as_int(une_result result)
{
  assert(result.type == UNE_RT_FLT);
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = (une_int)result.value._flt
  };
}

/*
Convert to FLT.
*/
une_result une_datatype_flt_as_flt(une_result result)
{
  assert(result.type == UNE_RT_FLT);
  return result;
}

/*
Convert to STR.
*/
une_result une_datatype_flt_as_str(une_result result)
{
  assert(result.type == UNE_RT_FLT);
  wchar_t *out = malloc(UNE_SIZE_NUM_TO_STR_LEN*sizeof(*out));
  verify(out);
  swprintf(out, UNE_SIZE_NUM_TO_STR_LEN, UNE_PRINTF_UNE_FLT, result.value._flt);
  return (une_result){
    .type = UNE_RT_STR,
    .value._wcs = out
  };
}

/*
Print a text representation to file.
*/
void une_datatype_flt_represent(FILE *file, une_result result)
{
  assert(result.type == UNE_RT_FLT);
  fwprintf(file, UNE_PRINTF_UNE_FLT, result.value._flt);
}

/*
Check for truth.
*/
une_int une_datatype_flt_is_true(une_result result)
{
  assert(result.type == UNE_RT_FLT);
  return result.value._flt == 0.0 ? 0 : 1;
}

/*
Check if subject is equal to comparison.
*/
une_int une_datatype_flt_is_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_FLT);
  if (comparison.type == UNE_RT_INT)
    return subject.value._flt == (une_flt)comparison.value._int;
  if (comparison.type == UNE_RT_FLT)
    return subject.value._flt == comparison.value._flt;
  return 0;
}

/*
Check if subject is greater than comparison.
*/
une_int une_datatype_flt_is_greater(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_FLT);
  if (comparison.type == UNE_RT_INT)
    return subject.value._flt > (une_flt)comparison.value._int;
  if (comparison.type == UNE_RT_FLT)
    return subject.value._flt > comparison.value._flt;
  return -1;
}

/*
Check if subject is greater than or equal to comparison.
*/
une_int une_datatype_flt_is_greater_or_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_FLT);
  if (comparison.type == UNE_RT_INT)
    return subject.value._flt >= (une_flt)comparison.value._int;
  if (comparison.type == UNE_RT_FLT)
    return subject.value._flt >= comparison.value._flt;
  return -1;
}

/*
Check is subject is less than comparison.
*/
une_int une_datatype_flt_is_less(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_FLT);
  if (comparison.type == UNE_RT_INT)
    return subject.value._flt < (une_flt)comparison.value._int;
  if (comparison.type == UNE_RT_FLT)
    return subject.value._flt < comparison.value._flt;
  return -1;
}

/*
Check if subject is less than or equal to comparison.
*/
une_int une_datatype_flt_is_less_or_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_FLT);
  if (comparison.type == UNE_RT_INT)
    return subject.value._flt <= (une_flt)comparison.value._int;
  if (comparison.type == UNE_RT_FLT)
    return subject.value._flt <= comparison.value._flt;
  return -1;
}

/*
Add right to left.
*/
une_result une_datatype_flt_add(une_result left, une_result right)
{
  assert(left.type == UNE_RT_FLT);
  if (right.type == UNE_RT_INT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = left.value._flt + (une_flt)right.value._int
    };
  if (right.type == UNE_RT_FLT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = left.value._flt + right.value._flt
    };
  return une_result_create(UNE_RT_ERROR);
}

/*
Subtract right from left.
*/
une_result une_datatype_flt_sub(une_result left, une_result right)
{
  assert(left.type == UNE_RT_FLT);
  if (right.type == UNE_RT_INT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = left.value._flt - (une_flt)right.value._int
    };
  if (right.type == UNE_RT_FLT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = left.value._flt - right.value._flt
    };
  return une_result_create(UNE_RT_ERROR);
}

/*
Multiply left by right.
*/
une_result une_datatype_flt_mul(une_result left, une_result right)
{
  assert(left.type == UNE_RT_FLT);
  if (right.type == UNE_RT_INT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = left.value._flt * (une_flt)right.value._int
    };
  if (right.type == UNE_RT_FLT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = left.value._flt * right.value._flt
    };
  return une_result_create(UNE_RT_ERROR);
}

/*
Divide left by right.
*/
une_result une_datatype_flt_div(une_result left, une_result right)
{
  assert(left.type == UNE_RT_FLT);
  
  /* Returns INFINITY on zero division. */
  if (right.type == UNE_RT_INT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = left.value._flt / (une_flt)right.value._int
    };
  if (right.type == UNE_RT_FLT) {
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = left.value._flt / right.value._flt
    };
  }
  return une_result_create(UNE_RT_ERROR);
}

/*
Floor divide left by right.
*/
une_result une_datatype_flt_fdiv(une_result left, une_result right)
{
  assert(left.type == UNE_RT_FLT);
  
  /* Return INFINITY on zero division. */
  if ((right.type == UNE_RT_INT && right.value._int == 0) || (right.type == UNE_RT_FLT && right.value._flt == 0.0))
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = INFINITY
    };
  
  if (right.type == UNE_RT_INT)
    return (une_result){
      .type = UNE_RT_INT,
      .value._int = (une_int)floor(left.value._flt / (une_flt)right.value._int)
    };
  if (right.type == UNE_RT_FLT)
    return (une_result){
      .type = UNE_RT_INT,
      .value._int = (une_int)floor(left.value._flt / right.value._flt)
    };
  return une_result_create(UNE_RT_ERROR);
}

/*
Get the modulus of left and right.
*/
une_result une_datatype_flt_mod(une_result left, une_result right)
{
  assert(left.type == UNE_RT_FLT);
  if (right.type == UNE_RT_INT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = fmod(left.value._flt, (une_flt)right.value._int)
    };
  if (right.type == UNE_RT_FLT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = fmod(left.value._flt, right.value._flt)
    };
  return une_result_create(UNE_RT_ERROR);
}

/*
Raise left to the power of right.
*/
une_result une_datatype_flt_pow(une_result left, une_result right)
{
  assert(left.type == UNE_RT_FLT);
  
  /* Returns NaN on unreal number. */
  if (right.type == UNE_RT_INT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = (une_flt)pow((double)left.value._flt, (double)right.value._int)
    };
  if (right.type == UNE_RT_FLT)
    return (une_result){
      .type = UNE_RT_FLT,
      .value._flt = (une_flt)pow((double)left.value._flt, (double)right.value._flt)
    };
  return une_result_create(UNE_RT_ERROR);
}

/*
Negate the result.
*/
une_result une_datatype_flt_negate(une_result result)
{
  assert(result.type == UNE_RT_FLT);
  return (une_result){
    .type = UNE_RT_FLT,
    .value._flt = -result.value._flt
  };
}
