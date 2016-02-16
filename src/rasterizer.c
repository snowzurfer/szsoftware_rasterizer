
#include <stdlib.h>
#include "rasterizer.h"
#include "dbg.h"

#define STATE_COUNT 2

typedef struct RenderTarget {
  uint8_t *target_location;
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
} CommandBuffer;

typedef struct PacketHeader {
  union {
    struct {
      uint32_t header : 8;
      uint32_t reserved : 24;
    };

    uint32_t value;
  };
} PacketHeader;

typedef struct CmdPacketSetVertexBuffer {
  PacketHeader packet_header;
  VertexBuffer *buffer;
} CmdPacketSetVertexBuffer;
typedef struct CmdPacketSetRenderTarget {
  PacketHeader packet_header;
  RenderTarget *target;
} CmdPacketSetRenderTarget;
typedef struct CmdPacketSetCullMode {
  PacketHeader packet_header;
  CullModeValues value;
} CmdPacketSetCullMode;
typedef struct CmdPacketSetWindingOrder{
  PacketHeader packet_header;
  WindingValues value;
} CmdPacketSetWindingOrder;
typedef struct CmdPacketDrawAuto{
  PacketHeader packet_header;
  uint32_t count;
} CmdPacketDrawAuto;



typedef struct State {
  uint32_t values[STATE_COUNT];
} State;
static State rasterizer_state;


CommandBuffer *CreateCommandBuffer(uint32_t size_bytes) {
  CommandBuffer *cmdbuffer = NULL;
  uint8_t *data = NULL;
  data = (uint8_t *)malloc(size_bytes);
  check_mem(data);

  cmdbuffer = (CommandBuffer *)malloc(sizeof(CommandBuffer));
  check_mem(cmdbuffer);

  cmdbuffer->start = data;
  cmdbuffer->current = data;
  cmdbuffer->end = data + size_bytes;

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

void DestroyCommandBuffer(CommandBuffer *cmdbuffer) {
  check(cmdbuffer != NULL, "cmdbuffer ptr passed is NULL");

  free(cmdbuffer->start);
  cmdbuffer->start = NULL;
  cmdbuffer->current = NULL;
  cmdbuffer->end = NULL;
  free(cmdbuffer);
  cmdbuffer = NULL;

  debug("Destroyed cmdbuffer");

error:
  ;
}