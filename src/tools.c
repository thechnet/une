/*
tools.c - Une
Updated 2021-06-13
*/

/* Header-specific includes. */
#include "tools.h"

/* Implementation-specific includes. */
#include <string.h>

/*
Attempt to allocate memory and crash immediately on fail.
*/
void *une_malloc(size_t size)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_ALLOC_COUNTER)
  une_alloc_count++;
  #endif
  void *p = malloc(size);
  if (p == NULL)
    ERR(L"Out of memory.");
  return p;
}

/*
Attempt to reallocate memory and crash immediately on fail.
*/
void *une_realloc(void *memory, size_t new_size)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_ALLOC_COUNTER)
  une_alloc_count++;
  #endif
  assert(memory != NULL);
  void *p = realloc(memory, new_size);
  if (p == NULL)
    ERR(L"Out of memory.");
  return p;
}

/*
Free memory.
*/
void une_free(void *memory)
{
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_ALLOC_COUNTER)
  une_alloc_count--;
  #endif
  assert(memory != NULL);
  free(memory);
}

/*
Convert a wchar_t string into a une_int integer.
This function does no error-checking and should only be used in a setting where
we already know the string is an integer number.
*/
une_int wcs_to_une_int(wchar_t *str)
{
  une_int out = 0;
  for (size_t i=0; str[i] != L'\0'; i++) {
    if (str[i] < L'0' || str[i] > L'9')
      return 0;
    out = 10 * out + str[i] - L'0';
  }
  return out;
}

/*
Convert a wchar_t string into a une_flt floating point number.
This function does no error-checking and should only be used in a setting where
we already know the string is a floating point number.
*/
une_flt wcs_to_une_flt(wchar_t *str)
{
  une_flt out = 0;
  for (int var=1, i=0; str[i] != L'\0'; i++) {
    if ((str[i] < L'0' || str[i] > L'9') /* Not a digit. */ && (str[i] != L'.' || var != 1))
      return 0; /* Second dot. */
    if (var != 1)
      var *= 10;
    if (str[i] == L'.') {
      var *= 10;
      i++;
    }
    out = (var != 1 ? 1 : 10) * out + ((double)(str[i] - L'0')) / var;
  }
  return out;
}

/*
Create a wchar_t string containing the contents of 'str'.
*/
wchar_t *str_to_wcs(char *str)
{
  size_t len = strlen(str);
  wchar_t *wcs = une_malloc((len+1) * sizeof(*wcs));
  swprintf(wcs, (len+1)*sizeof(*wcs), L"%hs", str);
  return wcs;
}

/*
Duplicate a char string.
*/
char *str_dup(char *src)
{
  if (src == NULL)
    return NULL;
  size_t len = strlen(src);
  char *new = une_malloc((len+1)*sizeof(*new));
  strcpy(new, src);
  return new;
}

/*
Duplicate a wchar_t string.
*/
wchar_t *wcs_dup(wchar_t *src)
{
  if (src == NULL)
    return NULL;
  size_t len = wcslen(src);
  wchar_t *new = une_malloc((len+1)*sizeof(*new));
  wcscpy(new, src);
  return new;
}
