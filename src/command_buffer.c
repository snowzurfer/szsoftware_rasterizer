
#include "command_buffer.h"
#include "dbg.h"

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

int32_t rtResetCmdBuffer(CmdBuffer *cmdbuff) {
  check(cmdbuff != NULL, "rtResetCmdBuffer(): cmdbuff ptr passed is NULL");

  CmdBufferInt *internal_cmdbuff = (CmdBufferInt *)cmdbuff;
  
  internal_cmdbuff->start = NULL;
  internal_cmdbuff->current = NULL;
  internal_cmdbuff->end = NULL;
  internal_cmdbuff->prev_buffer = NULL;

  debug("rtResetCmdBuffer(): Reseted cmdbuff");

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

void cmdBufQueueReset(CmdBuffersQueue *queue) {
  while (queue->lenght) {
    CmdBufferInt *buffer_to_free = cmdBufQueuePop(queue);
    if (buffer_to_free != NULL) {
      rtResetCmdBuffer((CmdBuffer *)buffer_to_free);
    }
  }

  queue->front = NULL;
  queue->rear = NULL;
  queue->lenght = 0;

  debug("cmdBufQueueReset(): cleared a queue");
}
