/* Private header file */

#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include <assert.h>
#include "rasterizer.h"

typedef struct IndexBufferInt {
  uint32_t *data;
  uint32_t indices_num;
} IndexBufferInt;
static_assert(sizeof(IndexBuffer) >= sizeof(IndexBufferInt),
  "Size of IndexBuffer must be >= the size of IndexBufferInt");

#endif
