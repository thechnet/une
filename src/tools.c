/*
tools.c - Une
Modified 2021-07-14
*/

/* Header-specific includes. */
#include "tools.h"

/* Implementation-specific includes. */
#include <string.h>
#include <errno.h>

/*
Convert a wchar_t string into a une_int integer.
This function does no error-checking and should only be used in a setting where
we already know the string is an integer number.
*/
une_int une_wcs_to_une_int(wchar_t *str)
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
une_flt une_wcs_to_une_flt(wchar_t *str)
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
Create a wchar_t string from a char string.
*/
wchar_t *une_str_to_wcs(char *str)
{
  size_t size = strlen(str)+1 /* NUL. */;
  wchar_t *wcs = malloc(size*sizeof(*wcs));
  mbstowcs(wcs, str, size);
  if (errno == EILSEQ) {
    free(wcs);
    return NULL;
  }
  return wcs;
}

/*
Create a char string from a wchar_t string.
*/
char *une_wcs_to_str(wchar_t *wcs)
{
  size_t size = wcslen(wcs)+1 /* NUL. */;
  char *str = malloc(size*sizeof(*str));
  wcstombs(str, wcs, size);
  if (errno == EILSEQ) {
    free(str);
    return NULL;
  }
  return str;
}

/*
Opens a UTF-8 file at 'path' and returns its text contents as wchar_t string.
*/
wchar_t *une_file_read(char *path)
{
  FILE *f = fopen(path, UNE_FOPEN_RFLAGS);
  if (f == NULL)
    return NULL;
  size_t text_size = UNE_SIZE_FILE_BUFFER;
  wchar_t *text = malloc(text_size*sizeof(*text));
  size_t cursor = 0;
  wint_t c; /* DOC: Can represent any Unicode character + WEOF (!).
               This is important when using fgetwc(), as otherwise,
               WEOF will overflow and be indistinguishable from an
               actual character. */
  while (true) {
    c = fgetwc(f);
    if (c == L'\r')
      continue;
    if (cursor >= text_size-1) { /* NUL. */
      text_size *= 2;
      text = realloc(text, text_size *sizeof(*text));
    }
    if (c == WEOF)
      break;
    text[cursor] = c;
    cursor++;
  }
  fclose(f);
  text[cursor] = L'\0';
  return text;
}
