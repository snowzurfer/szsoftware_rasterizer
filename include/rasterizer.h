
#ifndef RASTERIZER_H
#define RASTERIZER_H

#include <stdint.h>

typedef struct Device {
  uint32_t opaque_words[1024U];
} Device;
typedef struct RenderTarget {
 uint32_t opaque_words[1024U]; 
} RenderTarget;
typedef struct VertexBuffer {
 uint32_t opaque_words[1024U]; 
} VertexBuffer;
typedef struct CmdBuffer {
 uint32_t opaque_words[1024U]; 
} CmdBuffer;
typedef struct IndexBuffer {
 uint32_t opaque_words[1024U]; 
} IndexBuffer;

typedef enum WindingValues {
  RAST_WINDING_ORDER_CW,
  RAST_WINDING_ORDER_CCW,
} WindingValues;
typedef enum CullModeValues {
  RAST_CULL_MODE_FRONT,
  RAST_CULL_MODE_BACK,
  RAST_CULL_MODE_FRONT_AND_BACK,
} CullModeValues;


int32_t rtInitDevice(Device *device);

int32_t rtResetDevice(Device *device);

int32_t rtInitCmdBuffer(CmdBuffer *cmdbuff, void *data, uint32_t size_bytes);

int32_t rtResetCmdBuffer(CmdBuffer *cmdbuff);

int32_t rtInitVertexBuffer(VertexBuffer *vbuff, void *data, uint32_t size_data,
                           uint32_t size_element);

int32_t rtResetVertexBuffer(VertexBuffer *vbuff);

/* Index buffer is made of uint32s */ 
int32_t rtInitIndexBuffer(IndexBuffer *ibuff, uint32_t *data, uint32_t size_data);

int32_t rtResetIndexBuffer(IndexBuffer *ibuff);

int32_t rtInitRenderTarget(RenderTarget *target, void *data, uint32_t width,
                           uint32_t height);

int32_t rtResetRenderTarget(RenderTarget *target);

int32_t rtSetRenderTarget(CmdBuffer *cmdbuff, RenderTarget *target);

int32_t rtSetVertexBuffer(CmdBuffer *cmdbuff, VertexBuffer *vbuff);

int32_t rtSetIndexBuffer(CmdBuffer *cmdbuff, IndexBuffer *ibuff);

int32_t rtSetWindingOrder(CmdBuffer *cmdbuff, WindingValues value);

int32_t rtSetCullMode(CmdBuffer *cmdbuff, CullModeValues value);

int32_t rtSetModelMat(CmdBuffer *cmdbuff, const float *mat);

int32_t rtSetViewMat(CmdBuffer *cmdbuff, const float *mat);

int32_t rtSetProjectionMat(CmdBuffer *cmdbuff, const float *mat);

int32_t rtDrawAuto(CmdBuffer *cmdbuff, uint32_t count);

/*Clear the currently set render target to black*/
int32_t rtClearRenderTarget(CmdBuffer *cmdbuff, RenderTarget *target);

int32_t rtSubmit(Device *device, CmdBuffer *cmdbuff);

int32_t rtParseCmdBuffers(Device *device);

#endif
