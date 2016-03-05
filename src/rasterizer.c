
#include <stdlib.h>
#include <assert.h>
#include "rasterizer.h"
#include "dbg.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "render_target.h"
#include "command_buffer.h"
#include "device.h"
#include "vec4.h"
#include "vec2.h"
#include "vec3.h"

typedef struct Triangle {
  kmVec3 vertices[3U];
} Triangle;

/*typedef struct Point2D {*/
  
/*}*/

typedef struct TrisQueue {
  Triangle *front;
  Triangle *back;
  Triangle *start;
  Triangle *end;
} TrisQueue;
static int32_t trisQueueInit(TrisQueue *queue, Triangle *tris_batch,
                             uint32_t lenght);
static Triangle *trisQueuePop(TrisQueue *queue);
static int32_t trisQueuePush(TrisQueue *queue, Triangle *tri);
static void trisQueueReset(TrisQueue *queue);
static uint32_t trisQueueGetSize(TrisQueue *queue);


static int32_t rtVertexShading(DeviceInt *device, uint32_t draw_num,
                               TrisQueue *tri_queue_out);
static int32_t rtPixelShading(DeviceInt *device, TrisQueue *tri_queue_in);
/*Default is CCW; c is the pt to test against; a -> b, a -> c*/
static float rtEdgeFunction(const kmVec2 *a, const kmVec2 *b,
                              const kmVec2 *c);
static void rtDrawTri(const Triangle *tri, RenderTargetInt *target);
static uint32_t rtU32Min3(uint32_t a, uint32_t b, uint32_t c);
static uint32_t rtU32Max3(uint32_t a, uint32_t b, uint32_t c);
static uint32_t rtU32Min(uint32_t a, uint32_t b);
static uint32_t rtU32Max(uint32_t a, uint32_t b);



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

  debug("rtSetCullMode(): Set cull mode %d in cmdbuff %p",
    value, (void *)cmdbuff);
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

int32_t rtSetModelMat(CmdBuffer *cmdbuff, const float *mat) {
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = internal_cmdbuff->end - internal_cmdbuff->current;
  check((sizeof(PacketSetModelMat) <= space_left), "rtSetModelMat():\
 the cmdbuff doesn't have enough space left for the new command");

  PacketSetModelMat *packet =
    (PacketSetModelMat *)internal_cmdbuff->current;
  packet->packet_header.header = PACK_TYPE_SETMODELMAT;
  packet->mat = (kmMat4 *)mat;

  internal_cmdbuff->current += sizeof(PacketSetModelMat);

  debug("rtSetModelMat(): Set mat %p in cmdbuff %p",
    (void *)mat, (void *)cmdbuff);
  return 0;

error:

  return -1;
}

int32_t rtSetViewMat(CmdBuffer *cmdbuff, const float *mat) {
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = internal_cmdbuff->end - internal_cmdbuff->current;
  check((sizeof(PacketSetViewMat) <= space_left), "rtSetViewMat():\
 the cmdbuff doesn't have enough space left for the new command");

  PacketSetViewMat *packet =
    (PacketSetViewMat *)internal_cmdbuff->current;
  packet->packet_header.header = PACK_TYPE_SETVIEWMAT;
  packet->mat = (kmMat4 *)mat;

  internal_cmdbuff->current += sizeof(PacketSetViewMat);

  debug("rtSetViewMat(): Set mat %p in cmdbuff %p",
    (void *)mat, (void *)cmdbuff);
  return 0;

error:

  return -1;
}

int32_t rtSetProjectionMat(CmdBuffer *cmdbuff, const float *mat) {
  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  /* Check that the cmd buffer has enough space for the new packet */
  int32_t space_left = internal_cmdbuff->end - internal_cmdbuff->current;
  check((sizeof(PacketSetProjectionMat) <= space_left), "rtSetProjectionMat():\
 the cmdbuff doesn't have enough space left for the new command");

  PacketSetProjectionMat *packet =
    (PacketSetProjectionMat *)internal_cmdbuff->current;
  packet->packet_header.header = PACK_TYPE_SETPROJECTIONMAT;
  packet->mat = (kmMat4 *)mat;

  internal_cmdbuff->current += sizeof(PacketSetProjectionMat);

  debug("rtSetProjectionMat(): Set mat %p in cmdbuff %p",
    (void *)mat, (void *)cmdbuff);
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

          internal_device->target = packet->target;
          break;
        }
        case PACK_TYPE_SETVERTExBUFFER: {
          PacketSetVertexBuffer *packet = (PacketSetVertexBuffer *)read_ptr;
          debug("Parsed vertex buffer packet, addr: %p", (void *)packet);

          packet_size = sizeof(PacketSetVertexBuffer);

          internal_device->vbuff = packet->buffer;
          break;
        }
        case PACK_TYPE_SETCULLMODE: {
          PacketSetCullMode *packet = (PacketSetCullMode *)read_ptr;
          debug("Parsed cull mode packet, value: %d", packet->value);

          packet_size = sizeof(PacketSetCullMode);

          internal_device->state.values[STATE_CULL_MODE] = packet->value;
          break;
        }
        case PACK_TYPE_SETWINDINGORDER: {
          PacketSetWindingOrder *packet = (PacketSetWindingOrder *)read_ptr;
          debug("Parsed winding order packet, value: %d", packet->value);

          packet_size = sizeof(PacketSetWindingOrder);

          internal_device->state.values[STATE_WINDING] = packet->value;
          break;
        }
        case PACK_TYPE_DRAWAUTOTYPE: {
          PacketDrawAuto *packet = (PacketDrawAuto *)read_ptr;
          debug("Parsed draw auto packet, count: %d", packet->count);

          packet_size = sizeof(PacketDrawAuto);

          /* Launch rendering */
          TrisQueue tris_fifo;
          rtVertexShading(internal_device, packet->count, &tris_fifo);
          rtPixelShading(internal_device, &tris_fifo);
          break;
        }
        case PACK_TYPE_SETINDEXBUFFER: {
          PacketSetIndexBuffer *packet = (PacketSetIndexBuffer *)read_ptr;
          debug("Parsed index buffer packet, addr: %p", (void *)packet);

          packet_size = sizeof(PacketSetIndexBuffer);
 
          internal_device->ibuff = packet->buffer;
          break;
        }
        case PACK_TYPE_SETMODELMAT: {
          PacketSetModelMat *packet = (PacketSetModelMat *)read_ptr;
          debug("Parsed model mat buffer packet, addr: %p", (void *)packet);

          packet_size = sizeof(PacketSetModelMat);

          internal_device->model_mat = *packet->mat;
          break;
        }
        case PACK_TYPE_SETPROJECTIONMAT: {
          PacketSetProjectionMat *packet = (PacketSetProjectionMat *)read_ptr;
          debug("Parsed proj. mat buffer packet, addr: %p", (void *)packet);

          packet_size = sizeof(PacketSetProjectionMat);

          internal_device->projection_mat = *packet->mat;
          break;
        }
        case PACK_TYPE_SETVIEWMAT: {
          PacketSetViewMat *packet = (PacketSetViewMat *)read_ptr;
          debug("Parsed view mat buffer packet, addr: %p", (void *)packet);

          packet_size = sizeof(PacketSetViewMat);

          internal_device->view_mat = *packet->mat;
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

int32_t rtVertexShading(DeviceInt *device, uint32_t draw_num,
                               TrisQueue *tri_queue_out) {
  uint32_t indices_num = device->ibuff->indices_num;

  /*Create the list of triangles first*/
  uint32_t tris_num = indices_num / 3U;
  Triangle *tris_batch =
    (Triangle *)malloc(sizeof(Triangle) * tris_num);
  check(tris_batch != NULL, "[INTERNAL] rtVertexShading(): couldn't allocate \
      memory for the batch of triangles!");

  trisQueueInit(tri_queue_out, tris_batch, tris_num);

  /*Calculate PVM matrix*/
  kmMat4 PVM;
  kmMat4Multiply(&PVM, &device->projection_mat, &device->view_mat);
  kmMat4Multiply(&PVM, &PVM, &device->model_mat);

  /*For all the indices */
  uint32_t tris_count = 0U;
  for(uint32_t i = 0U; i < indices_num; i++) {
    kmVec4 v0;
    uint32_t index_num = device->ibuff->data[i];
    kmVec4Fill(
        &v0,
        *(device->vbuff->data + (index_num * 3U)),
        *(device->vbuff->data + (index_num * 3U) + 1U),
        *(device->vbuff->data + (index_num * 3U) + 2U),
        1.f);

    /*Convert to NDC space*/
    kmVec4 v1;
    kmVec4MultiplyMat4(&v1, &v0, &PVM);

    
    /*TODO Clip triangles*/
    

    kmVec4Scale(&v0, &v1, (1.f / v1.z));
    kmVec3 vc;
    kmVec3Fill(&vc, v0.x, v0.y, v0.z);

    /* Convert to raster space */ 
    vc.x = (vc.x + 1.f) / 2.f * (float)device->target->width;
    vc.y = (1.f - vc.y) / 2.f * (float)device->target->height;

    
    /*Add this vertex to the triangle*/
    tris_batch[i / 3U].vertices[i % 3U] = vc;

    /*When triangle full*/
    if((i % 3U) == 2U) {
      /*Push into queue*/
      trisQueuePush(tri_queue_out, tris_batch + tris_count);
      tris_count ++;
    }
  }

  debug("rtVertexShading(): Completed");
  return 0;

error:

  debug("rtVertexShading(): Error, exiting");
  return -1;
  
}
int32_t rtPixelShading(DeviceInt *device, TrisQueue *tri_queue_in) {
  /*For all the triangles*/
  Triangle *tri = trisQueuePop(tri_queue_in);
  while(tri != NULL) {
    /*Render them  */
    rtDrawTri(tri, device->target);   

    tri = trisQueuePop(tri_queue_in);
  }

  trisQueueReset(tri_queue_in);

  debug("rtPixelShading(): Completed");
  return 0;
}

int32_t trisQueueInit(TrisQueue *tris_queue, Triangle *tris_batch, uint32_t lenght) {
  check_mem(tris_queue);

  tris_queue->end = tris_batch + lenght;
  tris_queue->start = tris_batch;
  tris_queue->front = tris_batch;
  tris_queue->back = tris_batch;

  debug("trisQueueInit(): Initialised queue addr: %p", (void *)tris_queue);
  return 0;

error:

  return -1;
}

Triangle *trisQueuePop(TrisQueue *queue) {
  if(queue->back >= queue->front) {
    debug("trisQueuePop(): the queue is empty!");
    return NULL;
  }

  Triangle *popped_triangle = queue->back;
  queue->back ++;

  debug("trisQueuePop(): Popped an item, addr: %p", (void *)popped_triangle);
  return popped_triangle;
}

int32_t trisQueuePush(TrisQueue *queue, Triangle *tri) {
  if(queue->front >= queue->end) {
    debug("trisQueuePush(): the queue is full!");
    return -1;
  }

  queue->front = tri;
  queue->front ++;

  debug("trisQueuePush(): Pushed an item, addr: %p", (void *)tri);
  return 0;
}

void trisQueueReset(TrisQueue *queue) {
  free(queue->start);

  queue->front = NULL;
  queue->back = NULL;
  queue->end = NULL;
  queue->start = NULL;

  debug("trisQueueReset(): cleared a queue");
}

uint32_t trisQueueGetSize(TrisQueue *queue) {
  return queue->back - queue->front;
}

float rtEdgeFunction(const kmVec2 *a, const kmVec2 *b, const kmVec2 *c) {
  return (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
}

void rtDrawTri(const Triangle *tri, RenderTargetInt *target) {
  /*Compute the triangle's AABB*/
  int32_t min_x = rtU32Min3(
      tri->vertices[0].x,
      tri->vertices[1].x,
      tri->vertices[2].x);
  int32_t min_y = rtU32Min3(
      tri->vertices[0].y,
      tri->vertices[1].y,
      tri->vertices[2].y);
  int32_t max_x = rtU32Max3(
      tri->vertices[0].x,
      tri->vertices[1].x,
      tri->vertices[2].x);
  int32_t max_y = rtU32Max3(
      tri->vertices[0].y,
      tri->vertices[1].y,
      tri->vertices[2].y);

  /*Clip against screen bounds*/
  min_x = rtU32Max(min_x, 0U);
  min_y = rtU32Max(min_y, 0U);
  max_x = rtU32Min(max_x, target->width - 1U);
  max_y = rtU32Min(max_y, target->height - 1U);

  /*Rasterize the triangle*/
  kmVec2 p; /*The pixel position when rasterizing*/
  kmVec2 v0, v1, v2; /*The vertices of the triangle, but only their XY comps*/
  kmVec2Fill(&v0, tri->vertices[0U].x, tri->vertices[0U].y);
  kmVec2Fill(&v1, tri->vertices[1U].x, tri->vertices[1U].y);
  kmVec2Fill(&v2, tri->vertices[2U].x, tri->vertices[2U].y);
  float area = rtEdgeFunction(&v0, &v1, &v2);
  
  for(uint32_t j = min_y; j  <= max_y; j++) {
    for(uint32_t i  = min_x; i <= max_x; i++) {
      kmVec2Fill(&p, i + 0.5f, j + 0.5f);
      /*TODO: for now assume CCW; in future, account for both cases*/
      float w0 = rtEdgeFunction(&v1, &v2, &p);     
      float w1 = rtEdgeFunction(&v2, &v0, &p);     
      float w2 = rtEdgeFunction(&v0, &v1, &p);     

      /*TODO: account for top and left edges*/

      /*If p is inside or on all the edges*/
      if(w0 >= 0.f && w1 >= 0.f && w2 >= 0.f) {
        /*Calculate the barycentric coordinates */
        w0 /= area;
        w1 /= area;
        w2 /= area;

        uint32_t u32_size = sizeof(uint32_t);
        target->location[u32_size * (j * target->width + i)] = 255U;
        target->location[u32_size * (j * target->width + i) + 1U] = 255U;
        target->location[u32_size * (j * target->width + i) + 2U] = 255U;
        target->location[u32_size * (j * target->width + i) + 3U] = 255U;
      }
    } /*p.x for*/ 
  } /*p.y for*/
}

uint32_t rtU32Min3(uint32_t a, uint32_t b, uint32_t c) {
  uint32_t min = a;

  if(b < min) {
    min = b;
  }
  if(c < min) {
    min = c;
  }

  return min;
}

uint32_t rtU32Max3(uint32_t a, uint32_t b, uint32_t c) {
  uint32_t max = a;

  if(b > max) {
    max = b;
  }
  if(c > max) {
    max = c;
  }

  return max;
}

uint32_t rtU32Min(uint32_t a, uint32_t b) {
  return ((a < b) ? a : b);
}
uint32_t rtU32Max(uint32_t a, uint32_t b) {
  return ((a > b) ? a : b);
}
