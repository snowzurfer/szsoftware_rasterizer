
#include "index_buffer.h"
#include "dbg.h"
#include <stdint.h>

int32_t rtInitIndexBuffer(IndexBuffer *ibuff, uint32_t *data,
                          uint32_t size_data) {
  check(ibuff != NULL, "rtInitIndexBuffer(): ibuff ptr passed is NULL");

  IndexBufferInt *internal_ibuff  = (IndexBufferInt *)ibuff;

  internal_ibuff->data = data;

  internal_ibuff->indices_num = size_data / sizeof(uint32_t);

#ifndef NDEBUG
  for (uint32_t i = 0; i < internal_ibuff->indices_num; i++) {
    printf("\tIndx %d: %d\n", i,
      internal_ibuff->data[i]);
  }
#endif

  debug("rtInitIndexBuffer(): Initialised index buffer, addr: %p",
    (void *)ibuff);

  return 0;

error:

  return -1;
}

int32_t rtResetIndexBuffer(IndexBuffer *ibuff) {
  check(ibuff != NULL, "rtResetIndexBuffer(): ptr passed is NULL");

  IndexBufferInt *internal_ibuff  = (IndexBufferInt *)ibuff;

  internal_ibuff->indices_num = 0U;
  internal_ibuff->data = NULL;

  debug("rtResetIndexBuffer(): Reseted index buffer");

  return 0;

error:
  
  return -1;
}

