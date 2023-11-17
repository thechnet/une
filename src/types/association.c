/*
association.c - Une
Modified 2023-11-17
*/

/* Header-specific includes. */
#include "association.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Create an empty une_association.
*/
une_association *une_association_create(void)
{
	une_association *association = malloc(sizeof(*association));
	verify(association);
	association->name = NULL;
	association->content = une_result_create(UNE_RT_VOID);
	return association;
}

/*
Free all members of a une_association.
*/
void une_association_free(une_association *association)
{
	if (association->name)
		free(association->name);
	une_result_free(association->content);
	free(association);
}
