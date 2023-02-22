/*
symbols.c - Une
Modified 2023-02-22
*/

/* Header-specific includes. */
#include "symbols.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Free all members of a une_association.
*/
void une_variable_free(une_association *variable)
{
  free(variable->name);
  une_result_free(variable->content);
  free(variable);
}
