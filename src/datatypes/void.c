/*
void.c - Une
Modified 2023-11-17
*/

/* Header-specific includes. */
#include "void.h"

/*
Print a text representation to file.
*/
void une_datatype_void_represent(FILE *file, une_result result)
{
	assert(result.type == UNE_RT_VOID);
	fwprintf(file, L"Void");
}

/*
Check for truth.
*/
une_int une_datatype_void_is_true(une_result result)
{
	assert(result.type == UNE_RT_VOID);
	return 0;
}

/*
Check if subject is equal to comparison.
*/
une_int une_datatype_void_is_equal(une_result subject, une_result comparison)
{
	assert(subject.type == UNE_RT_VOID);
	if (comparison.type == UNE_RT_VOID)
		return 1;
	return 0;
}
