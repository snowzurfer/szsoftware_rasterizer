
#ifndef RASTERIZER_H
#define RASTERIZER_H

#include <stdint.h>

typedef struct Rasterizer Rasterizer;
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
void RasterizerDestroy(Rasterizer *rast);

RenderTarget *CreateRenderTarget(
  int32_t widht,
  int32_t height,
  int32_t pitch,
  int32_t format);

void RasterizerSetRenderTarget(Rasterizer *rast,
  RenderTarget *target);

void RasterizerSetVertexBuffer(Rasterizer *rast, VertexBuffer *buffer);

void RasterizerSetWindingOrder(Rasterizer *rast, WindingValues value);

void RasterizerSetCullMode(Rasterizer *rast, CullModeValues value);

void RasterizerDrawAuto(int32_t count);

void RasterizerSubmit(Rasterizer *rast, CommandBuffer *buffer);

#endif