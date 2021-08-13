/*
stream.c - Une
Modified 2021-08-12
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
    .has_reached_end = false,
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
      .file = fopen(path, UNE_FOPEN_RFLAGS),
      .wchar = L'\0'
    }
  };
  assert(istream.data.wfile.file != NULL);
  return istream;
}

/*
Reset an array une_istream.
*/
void une_istream_array_reset(une_istream *istream)
{
  istream->has_reached_end = false;
  istream->index = -1;
}

/*
Reset a wfile une_istream.
*/
void une_istream_wfile_reset(une_istream *istream)
{
  istream->has_reached_end = false;
  istream->index = -1;
  istream->data.wfile.wchar = L'\0';
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
  if ((istream->index)+offset < 0 || (istream->index)+offset >= istream->data.array.size)
    return false;
  return true;
}

/*
Ensure a une_ostream is large enough for another item.
*/
bool une_ostream_grow_if_needed(une_ostream *ostream, ptrdiff_t offset)
{
  if ((ostream->index)+offset < 0)
    return false;
  while ((ostream->index)+offset >= ostream->array_size) {
    if (!ostream->allow_resize)
      return false;
    ostream->array_size *= 2;
    ostream->array = realloc(ostream->array, ostream->array_size*ostream->item_size);
  }
  return true;
}
