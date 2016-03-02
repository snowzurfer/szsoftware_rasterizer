/* Private header file */

#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include <assert.h>
#include "rasterizer.h"


typedef struct VertexBufferInt {
  float *data;
  uint32_t vert_num;
} VertexBufferInt;
static_assert(sizeof(VertexBuffer) >= sizeof(VertexBufferInt),
  "Size of VertexBuffer must be >= the size of VertexBufferInt");

#endif
