
#include <stdlib.h>
#include <assert.h>
#include "rasterizer.h"
#include "dbg.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "render_target.h"
#include "command_buffer.h"
#include "device.h"




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
