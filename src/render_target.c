
#include "render_target.h"
#include "dbg.h"
#include <stdint.h>
#include <stdlib.h>

int32_t rtInitRenderTarget(RenderTarget *target, uint32_t width,
                           uint32_t height) {
  RenderTargetInt *internal_target = NULL;
  check(target != NULL, "rtInitRenderTarget(): target ptr passed is NULL");

  check((width != 0 && height != 0), "rtCreateRenderTarget(): the width or \
                                     the height passed is zero");
  
  internal_target = (RenderTargetInt *)target;

  internal_target->size_bytes = sizeof(void *) * width * height;
  internal_target->location = (uint8_t *)malloc(internal_target->size_bytes);
  check_mem(internal_target->location);

  internal_target->height = height;
  internal_target->width = width;
  internal_target->pitch = sizeof(void *) * width;

  debug("rtInitRenderTarget(): Initialised target, addr: %p", (void *)target);
  
  return 0;

error:

  return -1;
}

int32_t rtClearRenderTarget(RenderTarget *target) {
  check(target != NULL, "rtClearRenderTarget(): target ptr passed is NULL");

  RenderTargetInt *internal_target = (RenderTargetInt *)target;
  
  /* Delete the image/buffer itself first */
  free(internal_target->location);

  debug("rtClearRenderTarget(): Cleared target");

  return 0;

error:

  return -1;
}

