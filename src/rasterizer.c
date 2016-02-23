
#include <stdlib.h>
#include <assert.h>
#include "rasterizer.h"
#include "dbg.h"

#define STATE_COUNT 2
#define STATE_WINDING 0
#define STATE_CULL_MODE 1

typedef struct RenderTarget {
  uint8_t *location;
  uint32_t size_bytes;
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
  /* Assume RBGA format; in future iterations it will be modifiable */
} RenderTarget;

typedef struct VertexBuffer {
  float *data;
  uint32_t vert_num;
} VertexBuffer;


/*              Command buffer-related structures             */
typedef struct CmdBuffer {
  uint8_t *current;
  uint8_t *start;
  uint8_t *end;
  struct CmdBuffer *prev_buffer; /* Used by the queue */
} CmdBuffer;

typedef struct CmdBuffersQueue {
  CmdBuffer *front;
  CmdBuffer *rear;
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
  VertexBuffer *buffer;
} PacketSetVertexBuffer;
#define PACK_TYPE_SETVERTExBUFFER 0
typedef struct PacketSetRenderTarget {
  PacketHeader packet_header;
  RenderTarget *target;
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


typedef struct DeviceState {
  uint32_t values[STATE_COUNT];
} DeviceState;

typedef struct DeviceInt {
  DeviceState state;
  CmdBuffersQueue cmdbuffer_queue;
} DeviceInt;
static_assert(sizeof(Device) >= sizeof(DeviceInt),
  "Size of Device must be >= size of DeviceInt");


/* Function prototypes */
static int32_t cmdBufQueueInit(CmdBuffersQueue *cmdbuffer_queue);
static CmdBuffer *cmdBufQueuePop(CmdBuffersQueue *cmdbuffer_queue);
static void cmdBufQueuePush(CmdBuffersQueue *cmdbuffer_queue,
                               CmdBuffer *cmdbuffer);
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

CmdBuffer *rtCreateCmdBuffer(uint32_t size_bytes) {
  CmdBuffer *cmdbuffer = NULL;
  uint8_t *data = NULL;
  data = (uint8_t *)malloc(size_bytes);
  check_mem(data);

  cmdbuffer = (CmdBuffer *)malloc(sizeof(CmdBuffer));
  check_mem(cmdbuffer);

  cmdbuffer->start = data;
  cmdbuffer->current = data;
  cmdbuffer->end = data + size_bytes;
  cmdbuffer->prev_buffer = NULL;

  debug("rtCreateCmdBuffer(): Created cmdbuffer, addr: %p", (void *)cmdbuffer);
  return cmdbuffer;

error:
  if (data != NULL) {
    free(data );
    data = NULL;
  }

  if (cmdbuffer != NULL) {
    free(cmdbuffer);
    cmdbuffer = NULL;
  }

  return NULL;
}

void rtDestroyCmdBuffer(CmdBuffer *cmdbuffer) {
  check(cmdbuffer != NULL, "rtDestroyCmdBuffer(): ptr passed is NULL");

  free(cmdbuffer->start);
  cmdbuffer->start = NULL;
  cmdbuffer->current = NULL;
  cmdbuffer->end = NULL;
  cmdbuffer->prev_buffer = NULL;
  free(cmdbuffer);
  cmdbuffer = NULL;

  debug("rtDestroyCmdBuffer(): Destroyed cmdbuffer");

error:
  ;
}

/* For this method, opted for a solution similar to what done on DX11:
 provide the API with an array of vertices which is copied. After this
 function call, there will exist 2 arrays of vertex data: the one provided
 by the user, and the one on the API side.
 The user should then delete his/her own buffer.
 */
VertexBuffer *rtCreateVertexBuffer(void *data, uint32_t size_data,
    uint32_t size_element) {
  VertexBuffer *buffer = NULL;

  check_mem(data);

  /* Allocate vbuffer structure and the data it points to one after the other */
  uint32_t vbuf_totalsize = sizeof(VertexBuffer) + size_data;
  buffer = (VertexBuffer *)malloc(vbuf_totalsize);
  check_mem(buffer);

  buffer->data = (float *)(((uint8_t *)buffer) + sizeof(VertexBuffer));
  memcpy(buffer->data, data, size_data);

  buffer->vert_num = size_data / size_element;

#ifndef NDEBUG
  for (uint32_t i = 0; i < size_data / sizeof(float); i += 3) {
    printf("\tVert %d: %f,%f,%f\n", i / 3,
      buffer->data[i],
      buffer->data[i + 1],
      buffer->data[i + 2]);
  }
#endif

  debug("rtCreateVertexBuffer(): Created vertex buffer, addr: %p", (void *)buffer);
  return buffer;

error:
  return NULL;
}

void rtDestroyVertexBuffer(VertexBuffer *buffer) {
  check(buffer != NULL, "rtDestroyVertexBuffer(): ptr passed is NULL");

  /* Delete all of the data in one go since it was allocated
   in one malloc() only */
  free(buffer);
  buffer = NULL;

  debug("rtDestroyVertexBuffer(): Destroyed vertex buffer");

error:
  ;

}

RenderTarget *rtCreateRenderTarget(uint32_t width, uint32_t height) {
  RenderTarget *target = NULL;

  check((width != 0 && height != 0), "rtCreateRenderTarget(): the width or \
                                     the height passed is zero");

  target = (RenderTarget *)malloc(sizeof(RenderTarget));
  check_mem(target);

  target->size_bytes = sizeof(void *) * width * height;
  target->location = (uint8_t *)malloc(target->size_bytes);
  check_mem(target->location);

  target->height = height;
  target->width = width;
  target->pitch = sizeof(void *) * width;

  debug("rtCreateRenderTarget(): Created target, addr: %p", (void *)target);
  return target;

error:
  if (target != NULL) {
    free(target);
    target = NULL;
  }

  return NULL;
}

void rtDestroyRenderTarget(RenderTarget *target) {
  check(target != NULL, "rtDestroyRenderTarget(): ptr passed is NULL");

  /* Delete the image/buffer itself first */
  free(target->location);
  free(target);
  target = NULL;

  debug("rtDestroyRenderTarget(): Destroyed target");

error:
  ;
}

int32_t rtSetRenderTarget(CmdBuffer *cmdbuffer, RenderTarget *target) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(PacketSetRenderTarget) <= space_left), "rtSetRenderTarget():\
 the cmdbuffer doesn't have enough space left for the new command");

  check(target != NULL, "rtSetRenderTarget(): target passed is NULL");

  PacketSetRenderTarget *packet =
    (PacketSetRenderTarget *)cmdbuffer->current;
  packet->packet_header.header = PACK_TYPE_SETRENDERTARGET;
  packet->target = target;

  cmdbuffer->current += sizeof(PacketSetRenderTarget);

  debug("rtSetRenderTarget(): Set render target %p in cmdbuffer %p",
    (void *)target, (void *)cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtSetVertexBuffer(CmdBuffer *cmdbuffer, VertexBuffer *buffer) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(PacketSetVertexBuffer) <= space_left), "rtSetVertexBuffer():\
 the cmdbuffer doesn't have enough space left for the new command");

  check(buffer != NULL, "rtSetVertexBuffer(): buffer passed is NULL");

  PacketSetVertexBuffer *packet =
    (PacketSetVertexBuffer *)cmdbuffer->current;
  packet->packet_header.header = PACK_TYPE_SETVERTExBUFFER;
  packet->buffer = buffer;

  cmdbuffer->current += sizeof(PacketSetVertexBuffer);

  debug("rtSetVertexBuffer(): Set vertex buffer %p in cmdbuffer %p",
    (void *)buffer, (void *)cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtSetWindingOrder(CmdBuffer *cmdbuffer, WindingValues value) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(PacketSetWindingOrder) <= space_left), "rtSetWindingOrder():\
 the cmdbuffer doesn't have enough space left for the new command");

  /* Should check for validity of the winding value passed */

  PacketSetWindingOrder *packet =
    (PacketSetWindingOrder *)cmdbuffer->current;
  packet->packet_header.header = PACK_TYPE_SETWINDINGORDER;
  packet->value = value;

  cmdbuffer->current += sizeof(PacketSetWindingOrder);

  debug("rtSetWindingOrder(): Set winding order %p in cmdbuffer %p",
    (void *)value, (void *)cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtSetCullMode(CmdBuffer *cmdbuffer, CullModeValues value) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(PacketSetCullMode) <= space_left), "rtSetCullMode():\
 the cmdbuffer doesn't have enough space left for the new command");

  /* Should check for validity of the cull mode value passed */

  PacketSetCullMode *packet =
    (PacketSetCullMode *)cmdbuffer->current;
  packet->packet_header.header = PACK_TYPE_SETCULLMODE;
  packet->value = value;

  cmdbuffer->current += sizeof(PacketSetCullMode);

  debug("rtSetCullMode(): Set cull mode %p in cmdbuffer %p",
    (void *)value, (void *)cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtDrawAuto(CmdBuffer *cmdbuffer, uint32_t count) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(PacketDrawAuto) <= space_left), "rtDrawAuto():\
 the cmdbuffer doesn't have enough space left for the new command");

  PacketDrawAuto *packet =
    (PacketDrawAuto *)cmdbuffer->current;
  packet->packet_header.header = PACK_TYPE_DRAWAUTOTYPE;
  packet->count = count;

  cmdbuffer->current += sizeof(PacketDrawAuto);

  debug("rtDrawAuto(): Set draw auto of %d in cmdbuffer %p",
    (uint32_t)count, (void *)cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtSubmit(Device *device, CmdBuffer *buffer) {
  check(device != NULL, "rtInitDevice(): device ptr passed is NULL");
  
  DeviceInt  *internal_device = (DeviceInt *)device;
  
  cmdBufQueuePush(&(internal_device->cmdbuffer_queue), buffer);

  debug("Submitted cmdbuffer %p to device %p",
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
    debug("Parsed cmdbuffer");
    CmdBuffer *buffer = cmdBufQueuePop(&(internal_device->cmdbuffer_queue));
    /* A NULL buffer might have been added for various reasons; if so,
     just skip ahead */
    if (buffer == NULL) {
      continue;
    }

    /* Print out contents of the cmdbuffer */

    /* Cast head ptr to header */
    uint8_t *read_ptr = buffer->start;
    while (read_ptr < buffer->current) {
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
      }

      read_ptr += packet_size;
    }

    buffer->current = buffer->start;

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

CmdBuffer *cmdBufQueuePop(CmdBuffersQueue *queue) {
  check(queue->lenght > 0, "cmdBufQueuePop(): the queue is empty!");

  CmdBuffer *popped_cmdbuffer = queue->front;

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

void cmdBufQueuePush(CmdBuffersQueue *queue, CmdBuffer *cmdbuffer) {
  cmdbuffer->prev_buffer = NULL;

  switch (queue->lenght) {
    case 0: {
      queue->front = cmdbuffer;
      queue->rear = cmdbuffer;
      break;
    }
    default: {
      queue->rear->prev_buffer = cmdbuffer;
      queue->rear = cmdbuffer;
    }
  }

  queue->lenght ++;

  debug("cmdBufQueuePush(): Pushed an item, addr: %p", (void *)cmdbuffer);
}

void cmdBufQueueClear(CmdBuffersQueue *queue) {
  while (queue->lenght) {
    CmdBuffer *buffer_to_free = cmdBufQueuePop(queue);
    if (buffer_to_free != NULL) {
      rtDestroyCmdBuffer(buffer_to_free);
    }
  }

  queue->front = NULL;
  queue->rear = NULL;
  queue->lenght = 0;

  debug("cmdBufQueueClear(): cleared a queue");
}
