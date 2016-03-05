/* Private header file */

#ifndef COMMAND_BUFFER_H 
#define COMMAND_BUFFER_H

#include <assert.h>
#include <stdint.h>
#include "rasterizer.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "render_target.h"
#include "mat4.h"


/*              Command buffer-related structures             */
typedef struct CmdBufferInt {
  uint8_t *current;
  uint8_t *start;
  uint8_t *end;
  struct CmdBufferInt *prev_buffer; /* Used by the queue */
} CmdBufferInt;
static_assert(sizeof(CmdBuffer) >= sizeof(CmdBufferInt),
  "Size of CmdBuffer must be >= the size of CmdBufferInt");

typedef struct CmdBuffersQueue {
  CmdBufferInt *front;
  CmdBufferInt *rear;
  uint32_t lenght;
} CmdBuffersQueue;


typedef struct PacketHeader {
  union {
    struct {
      uint32_t header : 8;
      uint32_t reserved : 24;
    };

    uint32_t value;
  };
} PacketHeader;

/* Packets type consts are very rough. A better method to store this information
 will be developed. */
typedef struct PacketSetVertexBuffer {
  PacketHeader packet_header;
  VertexBufferInt *buffer;
} PacketSetVertexBuffer;
#define PACK_TYPE_SETVERTExBUFFER 0
typedef struct PacketSetRenderTarget {
  PacketHeader packet_header;
  RenderTargetInt *target;
} PacketSetRenderTarget;
#define PACK_TYPE_SETRENDERTARGET 1U
typedef struct PacketSetCullMode {
  PacketHeader packet_header;
  CullModeValues value;
} PacketSetCullMode;
#define PACK_TYPE_SETCULLMODE 2U
typedef struct PacketSetWindingOrder{
  PacketHeader packet_header;
  WindingValues value;
} PacketSetWindingOrder;
#define PACK_TYPE_SETWINDINGORDER 3U
typedef struct PacketDrawAuto{
  PacketHeader packet_header;
  uint32_t count;
} PacketDrawAuto;
#define PACK_TYPE_DRAWAUTOTYPE 4U
typedef struct PacketSetIndexBuffer {
  PacketHeader packet_header;
  IndexBufferInt *buffer;
} PacketSetIndexBuffer;
#define PACK_TYPE_SETINDEXBUFFER 5U
typedef struct PacketSetModelMat {
  PacketHeader packet_header;
  kmMat4 *mat;
} PacketSetModelMat;
#define PACK_TYPE_SETMODELMAT 6U
typedef struct PacketSetProjectionMat {
  PacketHeader packet_header;
  kmMat4 *mat; 
} PacketSetProjectionMat;
#define PACK_TYPE_SETPROJECTIONMAT 7U
typedef struct PacketSetViewMat {
  PacketHeader packet_header;
  kmMat4 *mat; 
} PacketSetViewMat;
#define PACK_TYPE_SETVIEWMAT 8U

/* Function prototypes */
int32_t cmdBufQueueInit(CmdBuffersQueue *cmdbuffer_queue);
CmdBufferInt *cmdBufQueuePop(CmdBuffersQueue *cmdbuffer_queue);
void cmdBufQueuePush(CmdBuffersQueue *cmdbuffer_queue,
                     CmdBufferInt *cmdbuff);
void cmdBufQueueClear(CmdBuffersQueue *cmdbuffer_queue);


#endif
