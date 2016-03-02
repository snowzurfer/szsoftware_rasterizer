/* Private header file */

#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H 

#include <assert.h>
#include "rasterizer.h"

typedef struct RenderTargetInt {
  uint8_t *location;
  uint32_t size_bytes;
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
  /* Assume RBGA format; in future iterations it will be modifiable */
} RenderTargetInt;
static_assert(sizeof(RenderTarget) >= sizeof(RenderTargetInt),
  "Size of RenderTarget must be >= the size of RenderTargetInt");

#endif
