/*
result.h - Une
Updated 2021-04-24
*/

#ifndef UNE_RESULT_H
#define UNE_RESULT_H

#include "../primitive.h"
#include "../tools.h"
#include "node.h"

#pragma region une_result_type
typedef enum _une_result_type {
  UNE_RT_ERROR,
  UNE_RT_INT,
  UNE_RT_FLT,
  UNE_RT_STR,
  UNE_RT_LIST,
  UNE_RT_ID,
  UNE_RT_CONTINUE,
  UNE_RT_BREAK,
  UNE_RT_SIZE,
} une_result_type;
#pragma endregion une_result_type

#pragma region une_result
typedef struct _une_result {
  une_result_type type;
  une_value value;
} une_result;
#pragma endregion une_result

const wchar_t *une_result_type_to_wcs(une_result_type result_type);
une_int une_result_is_true(une_result result);
une_int une_results_are_equal(une_result left, une_result right);
wchar_t *une_result_to_wcs(une_result result);
void une_result_free(une_result result);
une_result une_result_copy(une_result src);

#endif /* !UNE_RESULT_H */
