
#include "thread_pool_c.h"
#include "rasterizer.h"
#include <stdio.h>
#include <stdlib.h>
#include "dbg.h"

int main() {
  int32_t rc = -1;

  Device device;
  rc = rtInitDevice(&device);
  check(rc != -1, "Device couldn't be initialised");

  CmdBuffer cmdbuff;
  /* The data will be used by the CmdBuffer structure to store the commands */
  uint8_t cmdbuff_data[4096U]; 
  rc = rtInitCmdBuffer(&cmdbuff, &cmdbuff_data, 4096U);
  check(rc != -1, "Cmdbuffer couldn't be initialised");

  uint32_t num_vertices = 3;
  uint32_t vertex_size = sizeof(float) * 3;
  float *vertices = (float *)malloc(vertex_size * num_vertices);
  for (uint32_t i = 0; i < 3 * num_vertices; i++) {
    vertices[i] = (float)i;
  }

  VertexBuffer vtxbuff;
  rc = rtInitVertexBuffer(&vtxbuff, vertices,
    vertex_size * num_vertices, vertex_size);
  check(rc != -1, "VBuffer couldn't be initialised");


  RenderTarget target;
  rc = rtInitRenderTarget(&target, 800U, 600U);
  check(rc != -1, "Target couldn't be initialised");

  rtSetRenderTarget(&cmdbuff, &target);
  rtSetVertexBuffer(&cmdbuff, &vtxbuff);
  rtSetWindingOrder(&cmdbuff, RAST_WINDING_ORDER_CCW);
  rtSetCullMode(&cmdbuff, RAST_CULL_MODE_BACK);
  rtDrawAuto(&cmdbuff, num_vertices);

  rtSubmit(&device, &cmdbuff);

  rtParseCmdBuffers(&device);
  
  rtClearRenderTarget(&target);
  free(vertices);
  rtClearVertexBuffer(&vtxbuff);
  rtClearCmdBuffer(&cmdbuff);
  rtClearDevice(&device);

  getc(stdin);
  
  return 0;

error:

  return -1;
};
