/*
builtin.c - Une
Modified 2021-09-15
*/

/* Header-specific includes. */
#include "builtin_functions.h"

/* Implementation-specific includes. */
#include <time.h>
#include <limits.h>
#include "tools.h"
#include "une.h"
#include "stream.h"
#include "datatypes/datatypes.h"

/*
Pointers to all built-in functions.
*/
une_builtin_fnptr une_builtin_functions[] = {
  #define __BUILTIN_FUNCTION(__id) &une_builtin_fn_##__id,
  UNE_ENUMERATE_BUILTIN_FUNCTIONS(__BUILTIN_FUNCTION)
  #undef __BUILTIN_FUNCTION
};

/*
String representations of built-in function names.
*/
const wchar_t *une_builtin_functions_as_strings[] = {
  #define __BUILTIN_AS_STRING(__id) L"" #__id,
  UNE_ENUMERATE_BUILTIN_FUNCTIONS(__BUILTIN_AS_STRING)
  #undef __BUILTIN_AS_STRING
};

/* The amount of parameters of every built-in function. */
const size_t une_builtin_functions_params_count[] = {
  1, /* put */
  1, /* print */
  1, /* int */
  1, /* flt */
  1, /* str */
  1, /* len */
  1, /* sleep */
  1, /* chr */
  1, /* ord */
  1, /* read */
  2, /* write */
  1, /* input */
  1, /* script */
  1, /* exist */
  2 /* split */
};

/*
*** Interface.
*/

/*
Get the number of parameters of a built-in function.
*/
size_t une_builtin_params_count(une_builtin_function function)
{
  assert(UNE_BUILTIN_FUNCTION_IS_VALID(function));
  return une_builtin_functions_params_count[function];
}

/*
Get the function pointer for a builtin-in function.
*/
une_builtin_fnptr une_builtin_function_to_fnptr(une_builtin_function function)
{
  assert(UNE_BUILTIN_FUNCTION_IS_VALID(function));
  return une_builtin_functions[function-1];
}

/*
Get a pointer to the built-in function matching the given string or NULL;
*/
une_builtin_function une_builtin_wcs_to_function(wchar_t *wcs)
{
  une_builtin_function fn = __UNE_BUILTIN_none__;
  for (int i=1; i<__UNE_BUILTIN_max__; i++)
    if (wcscmp(une_builtin_functions_as_strings[i-1], wcs) == 0) {
      fn = i;
      break;
    }
  return fn;
}

/*
Convert a une_builtin_function to its string representation.
*/
#ifdef UNE_DEBUG
const wchar_t *une_builtin_function_to_wcs(une_builtin_function function)
{
  assert(UNE_BUILTIN_FUNCTION_IS_VALID(function));
  return une_builtin_functions_as_strings[function-1];
}
#endif /* UNE_DEBUG */

/*
***** Built-in Functions.
*/

/*
Print a text representation of a une_result.
*/
__une_builtin_fn(put)
{
  une_builtin_param string = 0;
  une_datatype dt_string = UNE_DATATYPE_FOR_RESULT(args[string]);
  
  assert(dt_string.represent != NULL);
  dt_string.represent(stdout, args[string]);
  
  return une_result_create(UNE_RT_VOID);
}

/*
Print a text representation of a une_result, always adding a newline at the end.
*/
__une_builtin_fn(print)
{
  une_builtin_fn_put(error, is, call_node, args);
  
  putwc(L'\n', stdout);
  
  return une_result_create(UNE_RT_VOID);
}

/*
Convert une_result to UNE_RT_INT une_result and return it.
*/
__une_builtin_fn(int)
{
  une_builtin_param result = 0;
  une_datatype dt_result = UNE_DATATYPE_FOR_RESULT(args[result]);
  
  if (dt_result.as_int == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, UNE_BUILTIN_POS_OF_ARG(result));
    return une_result_create(UNE_RT_ERROR);
  }
  
  une_result result_as_int = dt_result.as_int(args[result]);
  if (result_as_int.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, UNE_BUILTIN_POS_OF_ARG(result));
    return une_result_create(UNE_RT_ERROR);
  }
  
  return result_as_int;
}

/*
Convert une_result to UNE_RT_FLT une_result and return it.
*/
__une_builtin_fn(flt)
{
  une_builtin_param result = 0;
  une_datatype dt_result = UNE_DATATYPE_FOR_RESULT(args[result]);
  
  if (dt_result.as_flt == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, UNE_BUILTIN_POS_OF_ARG(result));
    return une_result_create(UNE_RT_ERROR);
  }
  
  une_result result_as_flt = dt_result.as_flt(args[result]);
  if (result_as_flt.type == UNE_RT_ERROR) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, UNE_BUILTIN_POS_OF_ARG(result));
    return une_result_create(UNE_RT_ERROR);
  }
  
  return result_as_flt;
}

/*
Convert une_result to UNE_RT_STR une_result and return it.
*/
__une_builtin_fn(str)
{
  une_builtin_param result = 0;
  une_datatype dt_result = UNE_DATATYPE_FOR_RESULT(args[result]);
  
  if (dt_result.as_str == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, UNE_BUILTIN_POS_OF_ARG(result));
    return une_result_create(UNE_RT_ERROR);
  }
  
  une_result result_as_str = dt_result.as_str(args[result]);
  assert(result_as_str.type != UNE_RT_ERROR);
  return result_as_str;
}

/*
Get length of une_result and return it.
*/
__une_builtin_fn(len)
{
  une_builtin_param result = 0;
  une_datatype dt_result = UNE_DATATYPE_FOR_RESULT(args[result]);
  
  if (dt_result.get_len == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, UNE_BUILTIN_POS_OF_ARG(result));
    return une_result_create(UNE_RT_ERROR);
  }
  
  size_t result_len = dt_result.get_len(args[result]);
  
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = (une_int)result_len
  };
}

/*
Halt execution for a given amount of miliseconds.
*/
__une_builtin_fn(sleep)
{
  une_builtin_param result = 0;
  
  UNE_BUILTIN_VERIFY_ARG_TYPE(result, UNE_RT_INT);
  
  /* Halt execution. */
  une_sleep_ms(args[result].value._int);
  
  return une_result_create(UNE_RT_VOID);
}

/*
Convert a number to its corresponding one-character string.
*/
__une_builtin_fn(chr)
{
  une_builtin_param result = 0;
  
  UNE_BUILTIN_VERIFY_ARG_TYPE(result, UNE_RT_INT);
  
  if (args[result].value._int > WCHAR_MAX) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, UNE_BUILTIN_POS_OF_ARG(result));
    return une_result_create(UNE_RT_ERROR);
  }
  
  une_result chr = une_result_create(UNE_RT_STR);
  chr.value._wcs = malloc(2*sizeof(*chr.value._wcs));
  verify(chr.value._wcs);
  chr.value._wcs[0] = (wchar_t)args[result].value._int;
  chr.value._wcs[1] = L'\0';
  return chr;
}

/*
Convert a one-character string to its corresponding number.
*/
__une_builtin_fn(ord)
{
  une_builtin_param result = 0;
  
  /* Ensure input une_result_type is UNE_RT_STR and is only one character long. */
  if (args[result].type != UNE_RT_STR || wcslen(args[result].value._wcs) != 1) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, UNE_BUILTIN_POS_OF_ARG(result));
    return une_result_create(UNE_RT_ERROR);
  }

  une_result ord = une_result_create(UNE_RT_INT);
  ord.value._int = (une_int)args[result].value._wcs[0];
  return ord;
}

/*
Return the entire contents of a file as text.
*/
__une_builtin_fn(read)
{
  une_builtin_param file = 0;
  
  UNE_BUILTIN_VERIFY_ARG_TYPE(file, UNE_RT_STR);
  
  /* Check if file exists. */
  char *path = une_wcs_to_str(args[file].value._wcs);
  if (path == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, UNE_BUILTIN_POS_OF_ARG(file));
    return une_result_create(UNE_RT_ERROR);
  }
  if (!une_file_exists(path)) {
    free(path);
    *error = UNE_ERROR_SET(UNE_ET_FILE_NOT_FOUND, UNE_BUILTIN_POS_OF_ARG(file));
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Read file. */
  une_result str = une_result_create(UNE_RT_STR);
  str.value._wcs = une_file_read(path);
  assert(str.value._wcs != NULL);
  free(path);
  return str;
}

/*
Write text to a file.
*/
__une_builtin_fn(write)
{
  une_builtin_param file = 0;
  une_builtin_param text = 1;
  
  UNE_BUILTIN_VERIFY_ARG_TYPE(file, UNE_RT_STR);
  UNE_BUILTIN_VERIFY_ARG_TYPE(text, UNE_RT_STR);

  /* Create file. */
  char *path = une_wcs_to_str(args[file].value._wcs);
  if (path == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, UNE_BUILTIN_POS_OF_ARG(file));
    return une_result_create(UNE_RT_ERROR);
  }
  FILE *fp = fopen(path, UNE_FOPEN_WFLAGS);
  free(path);
  if (fp == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_FILE_NOT_FOUND, UNE_BUILTIN_POS_OF_ARG(file));
    return une_result_create(UNE_RT_ERROR);
  }

  /* Print text. */
  fputws(args[text].value._wcs, fp);
  fclose(fp);
  return une_result_create(UNE_RT_VOID);
}

/*
Get user input as string from the console.
*/
__une_builtin_fn(input)
{
  une_builtin_param prompt = 0;
  
  UNE_BUILTIN_VERIFY_ARG_TYPE(prompt, UNE_RT_STR);

  /* Get string. */
  une_datatype_str_represent(stdout, args[prompt]);
  wchar_t *instr = malloc(UNE_SIZE_FGETWS_BUFFER*sizeof(*instr));
  verify(instr);
  fgetws(instr, UNE_SIZE_FGETWS_BUFFER, stdin);
  size_t len = wcslen(instr);
  instr[--len] = L'\0'; /* Remove trailing newline. */

  /* Return result. */
  une_result str = une_result_create(UNE_RT_STR);
  str.value._wcs = malloc((len+1)*sizeof(*str.value._wcs));
  verify(str.value._wcs);
  wcscpy(str.value._wcs, instr);
  free(instr);
  return str;
}

/*
Run an external Une script.
*/
__une_builtin_fn(script)
{
  une_builtin_param script = 0;
  
  UNE_BUILTIN_VERIFY_ARG_TYPE(script, UNE_RT_STR);

  /* Check if file exists. */
  char *path = une_wcs_to_str(args[script].value._wcs);
  if (path == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, UNE_BUILTIN_POS_OF_ARG(script));
    return une_result_create(UNE_RT_ERROR);
  }
  if (!une_file_exists(path)) {
    free(path);
    *error = UNE_ERROR_SET(UNE_ET_FILE_NOT_FOUND, UNE_BUILTIN_POS_OF_ARG(script));
    return une_result_create(UNE_RT_ERROR);
  }

  /* Run script. */
  une_result out = une_run(true, path, NULL);
  free(path);
  return out;
}

/*
Check if a file or directory exists.
*/
__une_builtin_fn(exist)
{
  une_builtin_param path = 0;
  
  UNE_BUILTIN_VERIFY_ARG_TYPE(path, UNE_RT_STR);

  /* Check if file or folder exists. */
  char *path_str = une_wcs_to_str(args[path].value._wcs);
  if (path_str == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, UNE_BUILTIN_POS_OF_ARG(path));
    return une_result_create(UNE_RT_ERROR);
  }
  bool exists = une_file_or_folder_exists(path_str);
  free(path_str);
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = (une_int)exists
  };
}

/* Split a string into a list of substrings. */
UNE_OSTREAM_PUSHER(__une_builtin_split_push, une_result)
__une_builtin_fn(split)
{
  une_builtin_param string = 0;
  une_builtin_param delims = 1;
  
  UNE_BUILTIN_VERIFY_ARG_TYPE(string, UNE_RT_STR);
  UNE_BUILTIN_VERIFY_ARG_TYPE(delims, UNE_RT_LIST);
  
  /* Ensure every member of delims is of type UNE_RT_STR. */
  UNE_UNPACK_RESULT_LIST(args[delims], delims_p, delims_len);
  UNE_FOR_RESULT_LIST_ITEM(i, delims_len) {
    if (delims_p[i].type != UNE_RT_STR || wcslen(delims_p[i].value._wcs) <= 0) {
      *error = UNE_ERROR_SET(UNE_ET_TYPE, UNE_BUILTIN_POS_OF_ARG(delims));
      return une_result_create(UNE_RT_ERROR);
    }
  }

  /* Setup. */
  size_t tokens_amt = 0;
  une_result *tokens = malloc((UNE_SIZE_BIF_SPLIT_TKS+1)*sizeof(*tokens));
  verify(tokens);
  une_ostream out = une_ostream_create((void*)tokens, UNE_SIZE_BIF_SPLIT_TKS+1, sizeof(*tokens), true);
  tokens = NULL; /* This pointer can turn stale after pushing. */
  void (*push)(une_ostream*, une_result) = &__une_builtin_split_push;
  push(&out, une_result_create(UNE_RT_SIZE));
  /* Cache delimiter lengths for performance. */
  size_t *delim_lens = malloc(delims_len*sizeof(*delim_lens));
  verify(delim_lens);
  for (size_t i=0; i<delims_len; i++)
    delim_lens[i] = wcslen(delims_p[i+1].value._wcs);
  wchar_t *wcs = args[string].value._wcs;
  size_t wcs_len = wcslen(wcs);
  size_t last_token_end = 0;
  int left_for_match = 0;

  /* Create tokens. */
  for (size_t i=0; i<wcs_len; i++) { /* For each character. */
    for (size_t delim=0; delim<delims_len; delim++) { /* For each delimiter. */
      left_for_match = delim_lens[delim];
      for (size_t delim_i=0; delim_i<delim_lens[delim]; delim_i++) { /* For each character in the delimiter. */
        if (i+delim_i >= wcs_len || wcs[i+delim_i] != delims_p[delim+1].value._wcs[delim_i])
          break;
        left_for_match--;
      }
      if (left_for_match != 0 && i+1 == wcs_len) { /* Special case for end of string. */
        i++;
        left_for_match = 0;
      }
      if (left_for_match == 0) { /* All characters have been matched. */
        size_t last_token_end_cpy = last_token_end;
        size_t substr_len = i-last_token_end_cpy;
        i += delim_lens[delim]-1 /* Compensate for next i++. */;
        last_token_end = i+1; /* last_token_end is not incremented next loop. */
        /* Break if there was no token before this delimiter. */
        if (substr_len == 0)
          break;
        /* Create substring. */
        wchar_t *substr = malloc((substr_len+1)*sizeof(*substr));
        verify(substr);
        wmemcpy(substr, wcs+last_token_end_cpy, substr_len);
        substr[substr_len] = L'\0';
        /* Push substring. */
        push(&out, (une_result){
          .type = UNE_RT_STR,
          .value._wcs = substr
        });
        tokens_amt++;
        break;
      }
    }
  }

  /* Wrap up. */
  free(delim_lens);
  tokens = (une_result*)out.array; /* Reobtain up-to-date pointer. */
  tokens[0].value._int = tokens_amt;
  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)tokens
  };
}
