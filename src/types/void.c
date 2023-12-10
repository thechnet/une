/*
void.c - Une
Modified 2023-12-10
*/

/* Header-specific includes. */
#include "void.h"

/*
Print a text representation to file.
*/
void une_type_void_represent(FILE *file, une_result result)
{
	assert(result.kind == UNE_RK_VOID);
	fwprintf(file, L"Void");
}

/*
Check for truth.
*/
une_int une_type_void_is_true(une_result result)
{
	assert(result.kind == UNE_RK_VOID);
	return 0;
}

/*
Check if subject is equal to comparison.
*/
une_int une_type_void_is_equal(une_result subject, une_result comparison)
{
	assert(subject.kind == UNE_RK_VOID);
	if (comparison.kind == UNE_RK_VOID)
		return 1;
	return 0;
}
