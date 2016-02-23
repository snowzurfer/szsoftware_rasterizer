
#ifndef RASTERIZER_H
#define RASTERIZER_H

#include <stdint.h>

typedef struct Device {
  uint32_t opaque_words[1024U];
} Device;
typedef struct RenderTarget RenderTarget;
typedef struct VertexBuffer VertexBuffer;
typedef struct CmdBuffer CmdBuffer;
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


int32_t rtInitDevice(Device *device);

int32_t rtClearDevice(Device *device);

CmdBuffer *rtCreateCmdBuffer(uint32_t size_bytes);

void rtDestroyCmdBuffer(CmdBuffer *cmdbuffer);

/* Other options for the signature of this method:
 - VertexBuffer *rtCreate...(uint32_t size_elements, uint32_t size_bytes_vertex)
*/
VertexBuffer *rtCreateVertexBuffer(void *data, uint32_t size_data,
  uint32_t size_element);

void rtDestroyVertexBuffer(VertexBuffer *buffer);

RenderTarget *rtCreateRenderTarget(uint32_t widht, uint32_t height);

void rtDestroyRenderTarget(RenderTarget *target);

int32_t rtSetRenderTarget(CmdBuffer *cmdbuffer, RenderTarget *target);

int32_t rtSetVertexBuffer(CmdBuffer *cmdbuffer, VertexBuffer *buffer);

int32_t rtSetWindingOrder(CmdBuffer *cmdbuffer, WindingValues value);

int32_t rtSetCullMode(CmdBuffer *cmdbuffer, CullModeValues value);

int32_t rtDrawAuto(CmdBuffer *cmdbuffer, uint32_t count);

int32_t rtSubmit(Device *device, CmdBuffer *cmdbuffer);

int32_t rtParseCmdBuffers(Device *device);

#endif
