
#include "vertex_buffer.h"
#include "dbg.h"
#include <stdint.h>

/* For this method, opted for a solution similar to what done on DX11:
 provide the API with an array of vertices which is copied. After this
 function call, there will exist 2 arrays of vertex data: the one provided
 by the user, and the one on the API side.
 The user should then delete his/her own buffer.
 */
int32_t rtInitVertexBuffer(VertexBuffer *vbuff, void *data, uint32_t size_data,
                           uint32_t size_element) {
  check(vbuff != NULL, "rtInitVertexBuffer(): vbuff ptr passed is NULL");

  VertexBufferInt *internal_vbuff  = (VertexBufferInt *)vbuff;

  internal_vbuff->data = (float *)data;

  internal_vbuff->vert_num = size_data / size_element;

#ifndef NDEBUG
  for (uint32_t i = 0; i < size_data / sizeof(float); i += 3) {
    printf("\tVert %d: %f,%f,%f\n", i / 3,
      internal_vbuff->data[i],
      internal_vbuff->data[i + 1],
      internal_vbuff->data[i + 2]);
  }
#endif

  debug("rtInitVertexBuffer(): Initialised vertex buffer, addr: %p",
    (void *)vbuff);

  return 0;

error:

  return -1;
}

int32_t rtResetVertexBuffer(VertexBuffer *vbuff) {
  check(vbuff != NULL, "rtResetVertexBuffer(): ptr passed is NULL");

  VertexBufferInt *internal_vbuff  = (VertexBufferInt *)vbuff;

  internal_vbuff->vert_num = 0U;
  internal_vbuff->data = NULL;

  debug("rtResetVertexBuffer(): Reseted vertex buffer");

  return 0;

error:
  
  return -1;
}

