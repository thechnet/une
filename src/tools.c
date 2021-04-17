/*
tools.c - Une
Updated 2021-04-17
*/

#include "tools.h"

#pragma region wcs_to_int
int wcs_to_int(wchar_t *str)
{
  int out = 0;
  for(int i = 0; str[i] != L'\0'; i++)
  {
    if(str[i] < L'0' || str[i] > L'9') return 0;
    out = 10 * out + str[i] - L'0';
  }
  return out;
}
#pragma endregion wcs_to_int

#pragma region wcs_to_une_int
une_int wcs_to_une_int(wchar_t *str)
{
  une_int out = 0;
  for(int i = 0; str[i] != L'\0'; i++)
  {
    if(str[i] < L'0' || str[i] > L'9') return 0;
    out = 10 * out + str[i] - L'0';
  }
  return out;
}
#pragma endregion wcs_to_une_int

#pragma region wcs_to_float
float wcs_to_float(wchar_t *str)
{
  float out = 0;
  for(int var = 1, i = 0; str[i] != L'\0'; i++)
  {
    if((str[i] < L'0' || str[i] > L'9')        // Not a digit.
    && (str[i] != L'.' || var != 1)) return 0; // Second dot.
    if(var != 1) var *= 10;
    if(str[i] == L'.') {
      var *= 10;
      i++;
    }
    out = (var != 1 ? 1 : 10) * out + ((float)(str[i] - L'0')) / var;
  }
  return out;
}
#pragma endregion wcs_to_float

#pragma region wcs_to_une_flt
une_flt wcs_to_une_flt(wchar_t *str)
{
  une_flt out = 0;
  for(int var = 1, i = 0; str[i] != L'\0'; i++)
  {
    if((str[i] < L'0' || str[i] > L'9')        // Not a digit.
    && (str[i] != L'.' || var != 1)) return 0; // Second dot.
    if(var != 1) var *= 10;
    if(str[i] == L'.') {
      var *= 10;
      i++;
    }
    out = (var != 1 ? 1 : 10) * out + ((double)(str[i] - L'0')) / var;
  }
  return out;
}
#pragma endregion wcs_to_une_flt

#pragma region file_read
wchar_t *file_read(char *path)
{
  size_t text_size = UNE_SIZE_MEDIUM;
  wchar_t *text = malloc(text_size *sizeof(*text));
  if(text == NULL) WERR(L"Out of memory.");
  FILE *f = fopen(path, "r,ccs=UTF-8");
  if(f == NULL) WERR(L"File not found");
  size_t cursor = 0;
  wint_t c; /* Can represent any Unicode character + (!) WEOF.
               This is important when using fgetwc(), as otherwise,
               WEOF will overflow and be indistinguishable from an
               actual character.*/
  while(true)
  {
    c = fgetwc(f);
    if(c == L'\r') continue;
    if(cursor >= text_size-1) // NUL
    {
      text_size *= 2;
      wchar_t *_text = realloc(text, text_size *sizeof(*_text));
      if(_text == NULL) WERR(L"Out of memory.");
      text = _text;
    }
    if(c == WEOF) break;
    text[cursor] = c;
    cursor++;
  }
  fclose(f);
  text[cursor] = L'\0';
  return text;
}
#pragma endregion file_read

#pragma region str_to_wcs
wchar_t *str_to_wcs(char *str)
{
  size_t len = strlen(str);
  wchar_t *wcs = malloc((len + 1) * sizeof(*wcs));
  if(wcs == NULL) WERR(L"Out of memory.");
  swprintf(wcs, (len + 1) * sizeof(*wcs), L"%hs", str);
  return wcs;
}
#pragma endregion str_to_wcs