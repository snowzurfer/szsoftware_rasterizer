
#include <stdlib.h>
#include <assert.h>
#include "rasterizer.h"
#include "dbg.h"

#define STATE_COUNT 2
#define STATE_WINDING 0
#define STATE_CULL_MODE 1

typedef struct RenderTargetInt {
  uint8_t *location;
  uint32_t size_bytes;
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
  /* Assume RBGA format; in future iterations it will be modifiable */
} RenderTargetInt;
static_assert(sizeof(RenderTarget) >= sizeof(RenderTargetInt),
  "Size of RenderTarget must be >= the size of RenderTargetInt");

typedef struct VertexBufferInt {
  float *data;
  uint32_t vert_num;
} VertexBufferInt;
static_assert(sizeof(VertexBuffer) >= sizeof(VertexBufferInt),
  "Size of VertexBuffer must be >= the size of VertexBufferInt");

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

typedef struct IndexBufferInt {
  uint32_t *data;
  uint32_t indices_num;
} IndexBufferInt;
static_assert(sizeof(IndexBuffer) >= sizeof(IndexBufferInt),
  "Size of IndexBuffer must be >= the size of IndexBufferInt");

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
static const uint8_t PacketSetRenderTargetType = 0;
typedef struct PacketSetCullMode {
  PacketHeader packet_header;
  CullModeValues value;
} PacketSetCullMode;
#define PACK_TYPE_SETCULLMODE 2U
static const uint8_t PacketSetCullModeType = 2;
typedef struct PacketSetWindingOrder{
  PacketHeader packet_header;
  WindingValues value;
} PacketSetWindingOrder;
#define PACK_TYPE_SETWINDINGORDER 3U
static const uint8_t PacketSetWindingOrderType = 3;
typedef struct PacketDrawAuto{
  PacketHeader packet_header;
  uint32_t count;
} PacketDrawAuto;
#define PACK_TYPE_DRAWAUTOTYPE 4U
static const uint8_t PacketDrawAutoType = 4;
typedef struct PacketSetIndexBuffer {
  PacketHeader packet_header;
  IndexBufferInt *buffer;
} PacketSetIndexBuffer;
#define PACK_TYPE_SETINDEXBUFFER 5U


typedef struct DeviceState {
  uint32_t values[STATE_COUNT];
} DeviceState;

typedef struct DeviceInt {
  DeviceState state;
  CmdBuffersQueue cmdbuffer_queue;
} DeviceInt;
static_assert(sizeof(Device) >= sizeof(DeviceInt),
  "Size of Device must be >= the size of DeviceInt");


/* Function prototypes */
static int32_t cmdBufQueueInit(CmdBuffersQueue *cmdbuffer_queue);
static CmdBufferInt *cmdBufQueuePop(CmdBuffersQueue *cmdbuffer_queue);
static void cmdBufQueuePush(CmdBuffersQueue *cmdbuffer_queue,
                               CmdBufferInt *cmdbuff);
static void cmdBufQueueClear(CmdBuffersQueue *cmdbuffer_queue);


int32_t rtInitDevice(Device *device) {
  check(device != NULL, "rtInitDevice(): device ptr passed is NULL");

  DeviceInt  *internal_device = (DeviceInt *)device;
  internal_device->state.values[STATE_WINDING] = RAST_WINDING_ORDER_CCW;
  internal_device->state.values[STATE_CULL_MODE] = RAST_CULL_MODE_BACK;

  // Initialise the queue
  int32_t rc = cmdBufQueueInit(&(internal_device->cmdbuffer_queue));
  check(rc != -1, "rtInitDevice(): Couldn't initialise the cmdbuf queue");

  debug("rtInitDevice(): Initialised device, addr: %p", (void *)device);

  return 0;

error:

  return -1;
}

int32_t rtClearDevice(Device *device) {
  check(device != NULL, "rtClearDevice(): device ptr passed is NULL");

  DeviceInt  *internal_device = (DeviceInt *)device;
  
  /* Clear also the queue of cmdbuffers */
  cmdBufQueueClear(&(internal_device->cmdbuffer_queue));

  debug("rtClearDevice(): Destroyed device");

  return 0;

error:

  return -1;
}

int32_t rtInitCmdBuffer(CmdBuffer *cmdbuff, void *data, uint32_t size_bytes) {
  check(cmdbuff != NULL, "rtInitCmdBuffer(): cmdbuff ptr passed is NULL");

  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;

  internal_cmdbuff->start = (uint8_t *)data;
  internal_cmdbuff->current = (uint8_t *)data;
  internal_cmdbuff->end = ((uint8_t *)data) + size_bytes;
  internal_cmdbuff->prev_buffer = NULL;

  debug("rtInitCmdBuffer(): Initialised cmdbuff, addr: %p",
    (void *)cmdbuff);
  
  return 0;

error:

  return -1;
}

int32_t rtClearCmdBuffer(CmdBuffer *cmdbuff) {
  check(cmdbuff != NULL, "rtClearCmdBuffer(): cmdbuff ptr passed is NULL");

  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  internal_cmdbuff->start = NULL;
  internal_cmdbuff->current = NULL;
  internal_cmdbuff->end = NULL;
  internal_cmdbuff->prev_buffer = NULL;

  debug("rtClearCmdBuffer(): Cleared cmdbuff");

  return 0;

error:

  return -1;
}

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

int32_t rtClearVertexBuffer(VertexBuffer *vbuff) {
  check(vbuff != NULL, "rtClearVertexBuffer(): ptr passed is NULL");

  VertexBufferInt *internal_vbuff  = (VertexBufferInt *)vbuff;

  internal_vbuff->vert_num = 0U;
  internal_vbuff->data = NULL;

  debug("rtClearVertexBuffer(): Cleared vertex buffer");

  return 0;

error:
  
  return -1;
}

int32_t rtInitIndexBuffer(IndexBuffer *ibuff, uint32_t *data,
                          uint32_t size_data) {
  check(ibuff != NULL, "rtInitIndexBuffer(): ibuff ptr passed is NULL");

  IndexBufferInt *internal_ibuff  = (IndexBufferInt *)ibuff;

  internal_ibuff->data = data;

  internal_ibuff->indices_num = size_data / sizeof(uint32_t);

#ifndef NDEBUG
  for (uint32_t i = 0; i < internal_ibuff->indices_num; i++) {
    printf("\tIndx %d: %d\n", i,
      internal_ibuff->data[i]);
  }
#endif

  debug("rtInitIndexBuffer(): Initialised index buffer, addr: %p",
    (void *)ibuff);

  return 0;

error:

  return -1;
}

int32_t rtClearIndexBuffer(IndexBuffer *ibuff) {
  check(ibuff != NULL, "rtClearIndexBuffer(): ptr passed is NULL");

  IndexBufferInt *internal_ibuff  = (IndexBufferInt *)ibuff;

  internal_ibuff->indices_num = 0U;
  internal_ibuff->data = NULL;

  debug("rtClearIndexBuffer(): Cleared index buffer");

  return 0;

error:
  
  return -1;
}

int32_t rtInitRenderTarget(RenderTarget *target, uint32_t width,
                           uint32_t height) {
  RenderTargetInt *internal_target = NULL;
  check(target != NULL, "rtInitRenderTarget(): target ptr passed is NULL");

  check((width != 0 && height != 0), "rtCreateRenderTarget(): the width or \
                                     the height passed is zero");
  
  internal_target = (RenderTargetInt *)target;

  internal_target->size_bytes = sizeof(void *) * width * height;
  internal_target->location = (uint8_t *)malloc(internal_target->size_bytes);
  check_mem(internal_target->location);

  internal_target->height = height;
  internal_target->width = width;
  internal_target->pitch = sizeof(void *) * width;

  debug("rtInitRenderTarget(): Initialised target, addr: %p", (void *)target);
  
  return 0;

error:

  return -1;
}

int32_t rtClearRenderTarget(RenderTarget *target) {
  check(target != NULL, "rtClearRenderTarget(): target ptr passed is NULL");

  RenderTargetInt *internal_target = (RenderTargetInt *)target;
  
  /* Delete the image/buffer itself first */
  free(internal_target->location);

  debug("rtClearRenderTarget(): Cleared target");

  return 0;

error:

  return -1;
}

int32_t rtSetRenderTarget(CmdBuffer *cmdbuff, RenderTarget *target) {
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = internal_cmdbuff->end - internal_cmdbuff->current;
  check((sizeof(PacketSetRenderTarget) <= space_left), "rtSetRenderTarget():\
 the cmdbuff doesn't have enough space left for the new command");

  check(target != NULL, "rtSetRenderTarget(): target passed is NULL");

  PacketSetRenderTarget *packet =
    (PacketSetRenderTarget *)internal_cmdbuff->current;
  packet->packet_header.header = PACK_TYPE_SETRENDERTARGET;
  packet->target = (RenderTargetInt *)target;

  internal_cmdbuff->current += sizeof(PacketSetRenderTarget);

  debug("rtSetRenderTarget(): Set render target %p in cmdbuff %p",
    (void *)target, (void *)cmdbuff);
  
  return 0;

error:
  
  return -1;
}

int32_t rtSetVertexBuffer(CmdBuffer *cmdbuff, VertexBuffer *buffer) {
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = internal_cmdbuff->end - internal_cmdbuff->current;
  check((sizeof(PacketSetVertexBuffer) <= space_left), "rtSetVertexBuffer():\
 the cmdbuff doesn't have enough space left for the new command");

  check(buffer != NULL, "rtSetVertexBuffer(): buffer passed is NULL");

  PacketSetVertexBuffer *packet =
    (PacketSetVertexBuffer *)internal_cmdbuff->current;
  packet->packet_header.header = PACK_TYPE_SETVERTExBUFFER;
  packet->buffer = (VertexBufferInt *)buffer;

  internal_cmdbuff->current += sizeof(PacketSetVertexBuffer);

  debug("rtSetVertexBuffer(): Set vertex buffer %p in cmdbuff %p",
    (void *)buffer, (void *)cmdbuff);
  
  return 0;

error:
  
  return -1;
}

int32_t rtSetIndexBuffer(CmdBuffer *cmdbuff, IndexBuffer *buffer) {
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = internal_cmdbuff->end - internal_cmdbuff->current;
  check((sizeof(PacketSetIndexBuffer) <= space_left), "rtSetIndexBuffer():\
 the cmdbuff doesn't have enough space left for the new command");

  check(buffer != NULL, "rtSetIndexBuffer(): buffer passed is NULL");

  PacketSetIndexBuffer *packet =
    (PacketSetIndexBuffer *)internal_cmdbuff->current;
  packet->packet_header.header = PACK_TYPE_SETINDEXBUFFER;
  packet->buffer = (IndexBufferInt *)buffer;

  internal_cmdbuff->current += sizeof(PacketSetIndexBuffer);

  debug("rtSetIndexBuffer(): Set index buffer %p in cmdbuff %p",
    (void *)buffer, (void *)cmdbuff);
  
  return 0;

error:
  
  return -1;
}

int32_t rtSetWindingOrder(CmdBuffer *cmdbuff, WindingValues value) {
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = internal_cmdbuff->end - internal_cmdbuff->current;
  check((sizeof(PacketSetWindingOrder) <= space_left), "rtSetWindingOrder():\
 the cmdbuff doesn't have enough space left for the new command");

  /* Should check for validity of the winding value passed */

  PacketSetWindingOrder *packet =
    (PacketSetWindingOrder *)internal_cmdbuff->current;
  packet->packet_header.header = PACK_TYPE_SETWINDINGORDER;
  packet->value = value;

  internal_cmdbuff->current += sizeof(PacketSetWindingOrder);

  debug("rtSetWindingOrder(): Set winding order %p in cmdbuff %p",
    (void *)value, (void *)cmdbuff);
  return 0;

error:
  return -1;
}

int32_t rtSetCullMode(CmdBuffer *cmdbuff, CullModeValues value) {
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = internal_cmdbuff->end - internal_cmdbuff->current;
  check((sizeof(PacketSetCullMode) <= space_left), "rtSetCullMode():\
 the cmdbuff doesn't have enough space left for the new command");

  /* Should check for validity of the cull mode value passed */

  PacketSetCullMode *packet =
    (PacketSetCullMode *)internal_cmdbuff->current;
  packet->packet_header.header = PACK_TYPE_SETCULLMODE;
  packet->value = value;

  internal_cmdbuff->current += sizeof(PacketSetCullMode);

  debug("rtSetCullMode(): Set cull mode %p in cmdbuff %p",
    (void *)value, (void *)cmdbuff);
  return 0;

error:
  return -1;
}

int32_t rtDrawAuto(CmdBuffer *cmdbuff, uint32_t count) {
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = internal_cmdbuff->end - internal_cmdbuff->current;
  check((sizeof(PacketDrawAuto) <= space_left), "rtDrawAuto():\
 the cmdbuff doesn't have enough space left for the new command");

  PacketDrawAuto *packet =
    (PacketDrawAuto *)internal_cmdbuff->current;
  packet->packet_header.header = PACK_TYPE_DRAWAUTOTYPE;
  packet->count = count;

  internal_cmdbuff->current += sizeof(PacketDrawAuto);

  debug("rtDrawAuto(): Set draw auto of %d in cmdbuff %p",
    (uint32_t)count, (void *)cmdbuff);
  return 0;

error:
  return -1;
}

int32_t rtSubmit(Device *device, CmdBuffer *buffer) {
  check(device != NULL, "rtInitDevice(): device ptr passed is NULL");
  
  DeviceInt  *internal_device = (DeviceInt *)device;
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)buffer;
  
  cmdBufQueuePush(&(internal_device->cmdbuffer_queue), internal_cmdbuff);

  debug("Submitted cmdbuff %p to device %p",
    (void *)buffer, (void *)device);

  return 0;

error:

  return -1;
}

int32_t rtParseCmdBuffers(Device *device) {
  check(device != NULL, "rtInitDevice(): device ptr passed is NULL");
  
  DeviceInt  *internal_device = (DeviceInt *)device;
  
  /* Print out the contents for now*/
  while (internal_device->cmdbuffer_queue.lenght) {
    debug("Parsed cmdbuff");
    CmdBufferInt *internal_cmdbuff =
      cmdBufQueuePop(&(internal_device->cmdbuffer_queue));
    /* A NULL buffer might have been added for various reasons; if so,
     just skip ahead */
    if (internal_cmdbuff == NULL) {
      continue;
    }


    /* Print out contents of the cmdbuff */

    /* Cast head ptr to header */
    uint8_t *read_ptr = internal_cmdbuff->start;
    while (read_ptr < internal_cmdbuff->current) {
      PacketHeader *packet_header = (PacketHeader *)read_ptr;
      uint32_t packet_size = 0U;
      /* Determine the type of packet */
      switch (packet_header->header) {
        case PACK_TYPE_SETRENDERTARGET: {
          PacketSetRenderTarget *packet = (PacketSetRenderTarget *)read_ptr;
          debug("Parsed render target packet, addr: %p", (void *)packet);
          packet_size = sizeof(PacketSetRenderTarget);
          break;
        }
        case PACK_TYPE_SETVERTExBUFFER: {
          PacketSetVertexBuffer *packet = (PacketSetVertexBuffer *)read_ptr;
          debug("Parsed vertex buffer packet, addr: %p", (void *)packet);
          packet_size = sizeof(PacketSetVertexBuffer);
          break;
        }
        case PACK_TYPE_SETCULLMODE: {
          PacketSetCullMode *packet = (PacketSetCullMode *)read_ptr;
          debug("Parsed cull mode packet, value: %d", packet->value);
          packet_size = sizeof(PacketSetCullMode);
          break;
        }
        case PACK_TYPE_SETWINDINGORDER: {
          PacketSetWindingOrder *packet = (PacketSetWindingOrder *)read_ptr;
          debug("Parsed winding order packet, value: %d", packet->value);
          packet_size = sizeof(PacketSetWindingOrder);
          break;
        }
        case PACK_TYPE_DRAWAUTOTYPE: {
          PacketDrawAuto *packet = (PacketDrawAuto *)read_ptr;
          debug("Parsed draw auto packet, count: %d", packet->count);
          packet_size = sizeof(PacketDrawAuto);
          break;
        }
        case PACK_TYPE_SETINDEXBUFFER: {
          PacketSetIndexBuffer *packet = (PacketSetIndexBuffer *)read_ptr;
          debug("Parsed index buffer packet, addr: %p", (void *)packet);
          packet_size = sizeof(PacketSetIndexBuffer);
          break;
        }
      }

      read_ptr += packet_size;
    }

    internal_cmdbuff->current = internal_cmdbuff->start;

  }

  return 0;

error:

  return -1;
}

int32_t cmdBufQueueInit(CmdBuffersQueue *cmdbuffer_queue) {
  check_mem(cmdbuffer_queue);

  cmdbuffer_queue->lenght = 0;
  cmdbuffer_queue->front = NULL;
  cmdbuffer_queue->rear = NULL;

  debug("cmdBufQueueInit(): Initialised queue addr: %p", (void *)cmdbuffer_queue);
  return 0;

error:

  return -1;
}

CmdBufferInt *cmdBufQueuePop(CmdBuffersQueue *queue) {
  check(queue->lenght > 0, "cmdBufQueuePop(): the queue is empty!");

  CmdBufferInt *popped_cmdbuffer = queue->front;

  switch (queue->lenght) {
    case 0: {
      break;
    }
    case 1: {
      queue->front = NULL;
      queue->rear = NULL;
      queue->lenght = 0;
      break;
    }
    default: {
      queue->front = popped_cmdbuffer->prev_buffer;
      queue->lenght --;
    }
  }

  debug("cmdBufQueuePop(): Popped an item, addr: %p", (void *)popped_cmdbuffer);
  return popped_cmdbuffer;

error:

  return NULL;
}

void cmdBufQueuePush(CmdBuffersQueue *queue, CmdBufferInt *cmdbuff) {
  cmdbuff->prev_buffer = NULL;

  switch (queue->lenght) {
    case 0: {
      queue->front = cmdbuff;
      queue->rear = cmdbuff;
      break;
    }
    default: {
      queue->rear->prev_buffer = cmdbuff;
      queue->rear = cmdbuff;
    }
  }

  queue->lenght ++;

  debug("cmdBufQueuePush(): Pushed an item, addr: %p", (void *)cmdbuff);
}

void cmdBufQueueClear(CmdBuffersQueue *queue) {
  while (queue->lenght) {
    CmdBufferInt *buffer_to_free = cmdBufQueuePop(queue);
    if (buffer_to_free != NULL) {
      rtClearCmdBuffer((CmdBuffer *)buffer_to_free);
    }
  }

  queue->front = NULL;
  queue->rear = NULL;
  queue->lenght = 0;

  debug("cmdBufQueueClear(): cleared a queue");
}
