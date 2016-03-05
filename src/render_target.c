
#include "render_target.h"
#include "dbg.h"
#include <stdint.h>
#include <stdlib.h>

int32_t rtInitRenderTarget(RenderTarget *target, void *data, uint32_t width,
                           uint32_t height) {
  RenderTargetInt *internal_target = NULL;
  check(target != NULL, "rtInitRenderTarget(): target ptr passed is NULL");

  check((width != 0 && height != 0), "rtCreateRenderTarget(): the width or \
                                     the height passed is zero");
  check_mem(data);
  
  internal_target = (RenderTargetInt *)target;

  internal_target->size_bytes = sizeof(uint32_t *) * width * height;
  internal_target->location = (uint8_t *)data;

  internal_target->height = height;
  internal_target->width = width;
  internal_target->pitch = sizeof(uint32_t *) * width;

  debug("rtInitRenderTarget(): Initialised target, addr: %p", (void *)target);
  
  return 0;

error:

  return -1;
}

int32_t rtResetRenderTarget(RenderTarget *target) {
  check(target != NULL, "rtResetRenderTarget(): target ptr passed is NULL");

  RenderTargetInt *internal_target = (RenderTargetInt *)target;
  internal_target->location = NULL;
  internal_target->size_bytes= 0U;
  internal_target->width = 0U;
  internal_target->height = 0U;
  internal_target->pitch = 0U;
  
  debug("rtResetRenderTarget(): Reseted target");

  return 0;

error:

  return -1;
}

