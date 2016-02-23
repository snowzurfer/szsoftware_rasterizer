
#include "thread_pool_c.h"
#include "rasterizer.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  Device device;
  rtInitDevice(&device);
  CmdBuffer *cmdbuf = rtCreateCmdBuffer(4096U);

  uint32_t num_vertices = 3;
  uint32_t vertex_size = sizeof(float) * 3;
  float *vertices = (float *)malloc(vertex_size * num_vertices);
  for (uint32_t i = 0; i < 3 * num_vertices; i++) {
    vertices[i] = (float)i;
  }

  VertexBuffer *vtxbuff = rtCreateVertexBuffer(vertices,
    vertex_size * num_vertices, vertex_size);
  free(vertices);

  RenderTarget *target = rtCreateRenderTarget(800U, 600U);


  rtSetRenderTarget(cmdbuf, target);
  rtSetVertexBuffer(cmdbuf, vtxbuff);
  rtSetWindingOrder(cmdbuf, RAST_WINDING_ORDER_CCW);
  rtSetCullMode(cmdbuf, RAST_CULL_MODE_BACK);
  rtDrawAuto(cmdbuf, num_vertices);

  rtSubmit(&device, cmdbuf);

  rtParseCmdBuffers(&device);
  
  rtDestroyRenderTarget(target);
  rtDestroyVertexBuffer(vtxbuff);
  rtDestroyCmdBuffer(cmdbuf);
  rtClearDevice(&device);

  getc(stdin);
  return 0;
};
