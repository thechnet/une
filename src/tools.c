/*
tools.c - Une
Modified 2021-07-15
*/

/* Header-specific includes. */
#include "tools.h"

/* Implementation-specific includes. */
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

/*
Convert a wchar_t string into a une_int integer.
*/
bool une_wcs_to_une_int(wchar_t *wcs, une_int* dest)
{
  wchar_t *wcs_end = wcs+wcslen(wcs);
  wchar_t *int_end;
  *dest = wcstoll(wcs, &int_end, 10);
  return int_end == wcs_end ? true : false;
}

/*
Convert a wchar_t string into a une_flt floating pointer number.
*/
bool une_wcs_to_une_flt(wchar_t *wcs, une_flt* dest)
{
  wchar_t *wcs_end = wcs+wcslen(wcs);
  wchar_t *flt_end;
  *dest = wcstod(wcs, &flt_end);
  return flt_end == wcs_end ? true : false;
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
Check if a file exists.
*/
bool une_file_exists(char *path)
{
  struct stat path_stat;
  int does_not_exist = stat(path, &path_stat);
  return (does_not_exist || S_ISDIR(path_stat.st_mode)) ? false : true;
}

/*
Check if a file or folder exists.
*/
bool une_file_or_folder_exists(char *path)
{
  struct stat path_stat;
  int does_not_exist = stat(path, &path_stat);
  return does_not_exist ? false : true;
}

/*
Open a UTF-8 file at 'path' and return its text contents as wchar_t string.
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
