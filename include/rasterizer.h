
#ifndef RASTERIZER_H
#define RASTERIZER_H

#include <stdint.h>

typedef struct Device Device;
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


Device *rtCreateDevice();
void rtDestroyDevice(Device *device);

CommandBuffer *rtCreateCmdBuffer(uint32_t size_bytes);

void rtDestroyCmdBuffer(CommandBuffer *cmdbuffer);

RenderTarget *rtCreateRenderTarget(uint32_t widht, uint32_t height);

void rtDestroyRenderTarget(RenderTarget *target);

int32_t rtSetRenderTarget(CommandBuffer *cmdbuffer, RenderTarget *target);

int32_t rtSetVertexBuffer(CommandBuffer *cmdbuffer, VertexBuffer *buffer);

int32_t rtSetWindingOrder(CommandBuffer *cmdbuffer, WindingValues value);

int32_t rtSetCullMode(CommandBuffer *cmdbuffer, CullModeValues value);

int32_t rtDrawAuto(CommandBuffer *cmdbuffer, uint32_t count);

int32_t rtSubmit(Device *device, CommandBuffer *cmdbuffer);

#endif