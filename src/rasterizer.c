
#include <stdlib.h>
#include "rasterizer.h"
#include "dbg.h"

#define STATE_COUNT 2

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
typedef struct CommandBuffer {
  uint8_t *current;
  uint8_t *start;
  uint8_t *end;
  struct CommandBuffer *prev_buffer; /* Used by the queue */
} CommandBuffer;

typedef struct CommandBuffersQueue {
  CommandBuffer *front;
  CommandBuffer *rear;
  uint32_t lenght;
} CommandBuffersQueue;


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
typedef struct CmdPacketSetVertexBuffer {
  PacketHeader packet_header;
  VertexBuffer *buffer;
} CmdPacketSetVertexBuffer;
static const uint8_t PacketSetVertexBufferType = 1;
typedef struct CmdPacketSetRenderTarget {
  PacketHeader packet_header;
  RenderTarget *target;
} CmdPacketSetRenderTarget;
static const uint8_t PacketSetRenderTargetType = 0;
typedef struct CmdPacketSetCullMode {
  PacketHeader packet_header;
  CullModeValues value;
} CmdPacketSetCullMode;
static const uint8_t PacketSetCullModeType = 2;
typedef struct CmdPacketSetWindingOrder{
  PacketHeader packet_header;
  WindingValues value;
} CmdPacketSetWindingOrder;
static const uint8_t PacketSetWindingOrderType = 3;
typedef struct CmdPacketDrawAuto{
  PacketHeader packet_header;
  uint32_t count;
} CmdPacketDrawAuto;
static const uint8_t PacketDrawAutoType = 4;


typedef struct DeviceState {
  uint32_t values[STATE_COUNT];
} DeviceState;

typedef struct Device {
  DeviceState state;
  CommandBuffersQueue cmdbuffer_queue;
} Device;



/* Function prototypes */
static int32_t cmdBufQueueInit(CommandBuffersQueue *cmdbuffer_queue);
static CommandBuffersQueue *cmdBufQueuePop(CommandBuffersQueue *cmdbuffer_queue);
static int32_t cmdBufQueuePush(CommandBuffersQueue *cmdbuffer_queue,
                               CommandBuffer *cmdbuffer);
static int32_t cmdBufQueueClear(CommandBuffersQueue *cmdbuffer_queue);


Device *rtCreateDevice() {
  Device *device = NULL;

  device = (Device *)malloc(sizeof(Device));
  check_mem(device);

  // Initialise the queue
  int32_t rc = cmdBufQueueInit(&(device->cmdbuffer_queue));
  check(rc != -1, "rtCreateDevice(): Couldn't initialise the cmdbuf queue");

  debug("Created device, addr: %d", device);
  return device;

error:
  if (device != NULL) {
    free(device);
    device = NULL;
  }

  return NULL;
}

void rtDestroyDevice(Device *device) {
  check(device != NULL, "rtDestroyDevice(): device ptr passed is NULL");

  /* Clear also the queue of cmdbuffers */
  int32_t rc = cmdBufQueueClear(&(device->cmdbuffer_queue));
  check(rc != -1, "rtDestroyDevice(): couldn't clear cmdbuffer queue");

  free(device);
  device = NULL;

  debug("Destroyed device");

error:
  if (device != NULL) {
    free(device);
    device = NULL;
  }
}

CommandBuffer *rtCreateCmdBuffer(uint32_t size_bytes) {
  CommandBuffer *cmdbuffer = NULL;
  uint8_t *data = NULL;
  data = (uint8_t *)malloc(size_bytes);
  check_mem(data);

  cmdbuffer = (CommandBuffer *)malloc(sizeof(CommandBuffer));
  check_mem(cmdbuffer);

  cmdbuffer->start = data;
  cmdbuffer->current = data;
  cmdbuffer->end = data + size_bytes;
  cmdbuffer->prev_buffer = NULL;

  debug("Created cmdbuffer, addr: %d", cmdbuffer);
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

void rtDestroyCmdBuffer(CommandBuffer *cmdbuffer) {
  check(cmdbuffer != NULL, "rtDestroyCmdBuffer(): ptr passed is NULL");

  free(cmdbuffer->start);
  cmdbuffer->start = NULL;
  cmdbuffer->current = NULL;
  cmdbuffer->end = NULL;
  cmdbuffer->prev_buffer = NULL;
  free(cmdbuffer);
  cmdbuffer = NULL;

  debug("Destroyed cmdbuffer");

error:
  ;
}

RenderTarget *rtCreateRenderTarget(uint32_t width, uint32_t height) {
  RenderTarget *target = NULL;

  check((width != 0 && height != 0), "rtCreateRenderTarget(): the width or \
                                     the height passed is zero");

  target = (RenderTarget *)malloc(sizeof(RenderTarget));
  check_mem(target);

  target->size_bytes = sizeof(uint32_t) * width * height;
  target->location = (uint8_t *)malloc(target->size_bytes);
  check_mem(target->location);

  target->height = height;
  target->width = width;
  target->pitch = sizeof(uint32_t) * width;

  debug("Created target, addr: %d", target);
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

  debug("Destroyed target");

error:
  ;
}

int32_t rtSetRenderTarget(CommandBuffer *cmdbuffer, RenderTarget *target) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(CmdPacketSetRenderTarget) <= space_left), "rtSetRenderTarget():\
 the cmdbuffer doesn't have enough space left for the new command");

  check(target != NULL, "rtSetRenderTarget(): target passed is NULL");

  CmdPacketSetRenderTarget *packet =
    (CmdPacketSetRenderTarget *)cmdbuffer->current;
  packet->packet_header.header = PacketSetRenderTargetType;
  packet->target = target;

  cmdbuffer->current += sizeof(CmdPacketSetRenderTarget);

  debug("Set render target %d in cmdbuffer %d", target, cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtSetVertexBuffer(CommandBuffer *cmdbuffer, VertexBuffer *buffer) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(CmdPacketSetVertexBuffer) <= space_left), "rtSetVertexBuffer():\
 the cmdbuffer doesn't have enough space left for the new command");

  check(buffer != NULL, "rtSetVertexBuffer(): buffer passed is NULL");

  CmdPacketSetVertexBuffer *packet =
    (CmdPacketSetVertexBuffer *)cmdbuffer->current;
  packet->packet_header.header = PacketSetVertexBufferType;
  packet->buffer = buffer;

  cmdbuffer->current += sizeof(CmdPacketSetVertexBuffer);

  debug("Set vertex buffer %d in cmdbuffer %d", buffer, cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtSetWindingOrder(CommandBuffer *cmdbuffer, WindingValues value) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(CmdPacketSetWindingOrder) <= space_left), "rtSetWindingOrder():\
 the cmdbuffer doesn't have enough space left for the new command");

  /* Should check for validity of the winding value passed */

  CmdPacketSetWindingOrder *packet =
    (CmdPacketSetWindingOrder *)cmdbuffer->current;
  packet->packet_header.header = PacketSetWindingOrderType;
  packet->value = value;

  cmdbuffer->current += sizeof(CmdPacketSetWindingOrder);

  debug("Set winding order %d in cmdbuffer %d", value, cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtSetCullMode(CommandBuffer *cmdbuffer, CullModeValues value) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(CmdPacketSetCullMode) <= space_left), "rtSetCullMode():\
 the cmdbuffer doesn't have enough space left for the new command");

  /* Should check for validity of the cull mode value passed */

  CmdPacketSetCullMode *packet =
    (CmdPacketSetCullMode *)cmdbuffer->current;
  packet->packet_header.header = PacketSetCullModeType;
  packet->value = value;

  cmdbuffer->current += sizeof(CmdPacketSetCullMode);

  debug("Set cull mode %d in cmdbuffer %d", value, cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtDrawAuto(CommandBuffer *cmdbuffer, uint32_t count) {
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = cmdbuffer->end - cmdbuffer->current;
  check((sizeof(CmdPacketDrawAuto) <= space_left), "rtDrawAuto():\
 the cmdbuffer doesn't have enough space left for the new command");

  CmdPacketDrawAuto *packet =
    (CmdPacketDrawAuto *)cmdbuffer->current;
  packet->packet_header.header = PacketDrawAutoType;
  packet->count = count;

  cmdbuffer->current += sizeof(CmdPacketDrawAuto);

  debug("Set draw auto of %d in cmdbuffer %d", count, cmdbuffer);
  return 0;

error:
  return -1;
}

int32_t rtSubmit(Device *device, CommandBuffer *buffer) {
  /* Print out the contents for now*/
  debug("rtSubmit(): function will print out contents of buffer as a start.\n\
Next iteration will do the actual work.");

  return 0;
}

int32_t cmdBufQueueInit(CommandBuffersQueue *cmdbuffer_queue) {

  return 0;
}

CommandBuffersQueue *cmdBufQueuePop(CommandBuffersQueue *cmdbuffer_queue) {

  return NULL;
}

int32_t cmdBufQueuePush(CommandBuffersQueue *cmdbuffer_queue,
  CommandBuffer *cmdbuffer) {

  return 0;
}

int32_t cmdBufQueueClear(CommandBuffersQueue *cmdbuffer_queue) {

  return 0;
}
