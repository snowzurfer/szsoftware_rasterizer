
#ifndef RASTERIZER_H
#define RASTERIZER_H

#include <stdint.h>

typedef struct RenderTarget RenderTarget;
typedef struct VertexBuffer VertexBuffer;
typedef struct CommandBuffer CommandBuffer;
typedef enum WindingValues {
  RAST_WINDING_ORDER_CW,
  RAST_WINDING_ORDER_CCW,
} WindingValues;
typedef enum CullModeValues {
  RAST_CULL_MODE_FRONT,
  RAST_CULL_MODE_BACK,
  RAST_CULL_MODE_FRONT_AND_BACK,
} CullModeValues;
typedef struct State State;


/* Rasterizer *RasterizerCreateRasterizer(); */
/* void RasterizerDestroy(Rasterizer *rast); */

CommandBuffer *CreateCommandBuffer(uint32_t size_bytes);

void DestroyCommandBuffer(CommandBuffer *cmdbuffer);

RenderTarget *CreateRenderTarget(
  uint32_t widht,
  uint32_t height);

void RasterizerSetRenderTarget(CommandBuffer *cmdbuffer,
  RenderTarget *target);

void RasterizerSetVertexBuffer(CommandBuffer *cmdbuffer, VertexBuffer *buffer);

void RasterizerSetWindingOrder(CommandBuffer *cmdbuffer, WindingValues value);

void RasterizerSetCullMode(CommandBuffer *cmdbuffer, CullModeValues value);

void RasterizerDrawAuto(CommandBuffer *cmdbuffer, uint32_t count);

void RasterizerSubmit(CommandBuffer *cmdbuffer);

#endif