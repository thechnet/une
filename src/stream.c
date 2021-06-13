/*
stream.c - Une
Modified 2021-06-13
*/

/* Header-specific includes. */
#include "stream.h"

/* Implementation-specific includes. */
#include "tools.h"

/*
Initialize an array une_istream struct.
*/
une_istream une_istream_array_create(void *array, size_t size)
{
  return (une_istream){
    .index = -1,
    .data.array = {
      .array = array,
      .size = size
    }
  };
}

/*
Initialize a une_ostream struct.
*/
une_ostream une_ostream_create(void *array, size_t array_size, size_t item_size, bool allow_resize)
{
  return (une_ostream){
    .index = -1,
    .array = array,
    .array_size = array_size,
    .item_size = item_size,
    .allow_resize = allow_resize
  };
}

/*
Initialize a wfile une_istream struct.
*/
une_istream une_istream_wfile_create(char *path)
{
  une_istream istream = (une_istream){
    .index = -1,
    .data.wfile = {
      .file = fopen(path, "r,ccs=UTF-8"),
      .wchar = L'\0'
    }
  };
  if (istream.data.wfile.file == NULL)
    ERR(L"File not found.");
  return istream;
}

/*
Reset an array une_istream.
*/
void une_istream_array_reset(une_istream *istream)
{
  istream->index = -1;
}

/*
Reset a wfile une_istream.
*/
void une_istream_wfile_reset(une_istream *istream)
{
  istream->index = -1;
  rewind(istream->data.wfile.file);
}

/*
Free all members of a wfile une_istream.
*/
void une_istream_wfile_free(une_istream stream)
{
  fclose(stream.data.wfile.file);
}

/*
Verify a position in an array une_istream.
*/
bool une_istream_array_verify_position(une_istream *istream, ptrdiff_t offset)
{
  if (istream->index+offset < 0 || istream->index+offset >= istream->data.array.size)
    return false;
  return true;
}

/*
Verify a position in a une_ostream.
*/
bool une_ostream_verify_position(une_ostream *ostream, ptrdiff_t offset)
{
  if (ostream->index+offset < 0)
    return false;
  while (ostream->index+offset >= ostream->array_size) {
    if (!ostream->allow_resize)
      return false;
    ostream->array_size *= 2;
    ostream->array = une_realloc(ostream->array, ostream->array_size*ostream->item_size);
  }
  return true;
}
