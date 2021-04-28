/*
tools.h - Une
Updated 2021-04-28
*/

#ifndef UNE_TOOLS_H
#define UNE_TOOLS_H

#include "primitive.h"
#include <stdio.h>
#include <string.h>

void *rmalloc(size_t size);
void *rrealloc(void *memory, size_t new_size);
une_int wcs_to_une_int (wchar_t *str);
une_flt wcs_to_une_flt (wchar_t *str);
wchar_t *file_read (char *path);
wchar_t *str_to_wcs (char *str);
wchar_t *wcs_dup (wchar_t *src);

#endif /* !UNE_TOOLS_H */
