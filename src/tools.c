/*
tools.c - Une
Updated 2021-06-04
*/

#include "tools.h"

#pragma region rmalloc
/* DOC:
Attemps to allocate memory and crashes immediately on fail.
*/
void *rmalloc(size_t size)
{
  void *p = malloc(size);
  if (p == NULL) WERR(L"Out of memory.");
  return p;
}
#pragma endregion rmalloc

#pragma region rrealloc
/* DOC:
Attemps to reallocate memory and crashes immediately on fail.
*/
void *rrealloc(void *memory, size_t new_size)
{
  void *p = realloc(memory, new_size);
  if (p == NULL) WERR(L"Out of memory.");
  return p;
}
#pragma endregion rmalloc

#pragma region wcs_to_une_int
/* DOC:
Converts a wchar_t string into a une_int integer.
This function does no error-checking and should only be used in a setting where
we already know the string is an integer number.
*/
une_int wcs_to_une_int(wchar_t *str)
{
  une_int out = 0;
  for (size_t i=0; str[i] != L'\0'; i++) {
    if (str[i] < L'0' || str[i] > L'9') return 0;
    out = 10 * out + str[i] - L'0';
  }
  return out;
}
#pragma endregion wcs_to_une_int

#pragma region wcs_to_une_flt
/* DOC:
Converts a wchar_t string into a une_flt floating point number.
This function does no error-checking and should only be used in a setting where
we already know the string is a floating point number.
*/
une_flt wcs_to_une_flt(wchar_t *str)
{
  une_flt out = 0;
  for (int var=1, i = 0; str[i] != L'\0'; i++) {
    if ((str[i] < L'0' || str[i] > L'9')        // Not a digit.
    && (str[i] != L'.' || var != 1)) return 0; // Second dot.
    if (var != 1) var *= 10;
    if (str[i] == L'.') {
      var *= 10;
      i++;
    }
    out = (var != 1 ? 1 : 10) * out + ((double)(str[i] - L'0')) / var;
  }
  return out;
}
#pragma endregion wcs_to_une_flt

#pragma region file_read
/* DOC:
Opens a UTF-8 file at 'path' and returns its text contents as wchar_t string.
*/
wchar_t *file_read(char *path, bool include_carriage_return)
{
  size_t text_size = UNE_SIZE_MEDIUM;
  wchar_t *text = rmalloc(text_size * sizeof(*text));
  FILE *f = fopen(path, "r,ccs=UTF-8");
  if (f == NULL) WERR(L"File not found");
  size_t cursor = 0;
  wint_t c; /* DOC: Can represent any Unicode character + (!) WEOF.
               This is important when using fgetwc(), as otherwise,
               WEOF will overflow and be indistinguishable from an
               actual character. */
  while (true) {
    c = fgetwc(f);
    if (c == L'\r' && !include_carriage_return) continue;
    if (cursor >= text_size-1) { // NUL
      text_size *= 2;
      wchar_t *_text = rrealloc(text, text_size *sizeof(*_text));
      text = _text;
    }
    if (c == WEOF) break;
    text[cursor] = c;
    cursor++;
  }
  fclose(f);
  text[cursor] = L'\0';
  return text;
}
#pragma endregion file_read

#pragma region str_to_wcs
/* DOC:
Creates a wchar_t string containing the contents of 'str'.
*/
wchar_t *str_to_wcs(char *str)
{
  size_t len = strlen(str);
  wchar_t *wcs = rmalloc((len + 1) * sizeof(*wcs));
  swprintf(wcs, (len + 1) * sizeof(*wcs), L"%hs", str);
  return wcs;
}
#pragma endregion str_to_wcs

#pragma region str_dup
/* DOC:
Duplicates a char string.
*/
char *str_dup(char *src)
{
  if (src == NULL) return NULL;
  size_t len = strlen(src);
  char *new = rmalloc((len+1)*sizeof(*new));
  strcpy(new, src);
  return new;
}
#pragma endregion str_dup

#pragma region wcs_dup
/* DOC:
Duplicates a wchar_t string.
*/
wchar_t *wcs_dup(wchar_t *src)
{
  if (src == NULL) return NULL;
  size_t len = wcslen(src);
  wchar_t *new = rmalloc((len+1)*sizeof(*new));
  wcscpy(new, src);
  return new;
}
#pragma endregion wcs_dup