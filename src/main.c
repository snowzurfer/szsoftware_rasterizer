
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

  uint32_t num_vertices = 3U;
  uint32_t vertex_size = sizeof(float) * 3U;
  float *vertices = (float *)malloc(vertex_size * num_vertices);
  vertices[0U] = -0.8f; vertices[1U] = -0.8f; vertices[2U] = 1.f;
  vertices[3U] = 0.f; vertices[4U] = 0.8f; vertices[5U] = 1.f;
  vertices[6U] = 0.8f; vertices[7U] = -0.8f; vertices[8U] = 1.f;

  VertexBuffer vtxbuff;
  rc = rtInitVertexBuffer(&vtxbuff, vertices,
    vertex_size * num_vertices, vertex_size);
  check(rc != -1, "VBuffer couldn't be initialised");

  uint32_t num_indices = 3U;
  uint32_t *indices = (uint32_t *)malloc(sizeof(uint32_t) * num_indices);
  for (uint32_t i = 0U; i < num_indices; i++) {
    indices[i] = i;
  }

  IndexBuffer indbuff;
  rc = rtInitIndexBuffer(&indbuff, indices, num_indices * sizeof(uint32_t));
  check(rc != -1, "IBuffer couldn't be initialised");

  uint32_t target_w = 800U;
  uint32_t target_h = 600U;
  RenderTarget target;
  uint8_t *target_data =
    (uint8_t *)malloc(sizeof(uint32_t) * target_w * target_h);
  rc = rtInitRenderTarget(&target, target_data, target_w, target_h);
  check(rc != -1, "Target couldn't be initialised");

  rtSetRenderTarget(&cmdbuff, &target);
  rtSetVertexBuffer(&cmdbuff, &vtxbuff);
  rtSetIndexBuffer(&cmdbuff, &indbuff);
  rtSetWindingOrder(&cmdbuff, RAST_WINDING_ORDER_CCW);
  rtSetCullMode(&cmdbuff, RAST_CULL_MODE_BACK);
  rtDrawAuto(&cmdbuff, num_vertices);

  rtSubmit(&device, &cmdbuff);

  rtParseCmdBuffers(&device);

  /*Render to ppm to test out rendering*/
  uint32_t u32_size = sizeof(uint32_t);
  FILE *of = fopen("render.ppm", "wb");
  fprintf(of, "P6\n%d %d\n255\n", target_w, target_h);
  for(uint32_t j = 0U; j < target_h; j++) {
    for(uint32_t i = 0U; i < target_w; i++) {
      static uint8_t colour[3U];
      colour[0U] = target_data[u32_size * (j * target_w + i)];
      colour[1U] = target_data[u32_size * (j * target_w + i) + 1U];
      colour[2U] = target_data[u32_size * (j * target_w + i) + 2U];
      fwrite(colour, 1U, 3U, of);
    }
  }

  fclose(of);
  
  rtClearRenderTarget(&target);
  free(target_data);
  free(indices);
  free(vertices);
  rtClearIndexBuffer(&indbuff);
  /*for (uint32_t i = 0U; i < 3U * num_vertices; i++) {*/
    /*vertices[i] = (float)i;*/
  /*}*/
  rtClearVertexBuffer(&vtxbuff);
  rtClearCmdBuffer(&cmdbuff);
  rtClearDevice(&device);

  /*getc(stdin);*/
  
  return 0;

error:

  return -1;
};
