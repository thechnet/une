/*
tools.h - Une
Updated 2021-04-17
*/

#ifndef UNE_TOOLS_H
#define UNE_TOOLS_H

#include "primitive.h"
#include <stdio.h>
#include <string.h>

int wcs_to_int(wchar_t *str);
une_int wcs_to_une_int(wchar_t *str);
float wcs_to_float(wchar_t *str);
une_flt wcs_to_une_flt(wchar_t *str);
wchar_t *file_read(char *path);
wchar_t *str_to_wcs(char *str);

#endif /* !UNE_TOOLS_H */