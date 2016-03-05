
#include "device.h"
#include "dbg.h"
#include <stdint.h>
#include "mat4.h"

int32_t rtInitDevice(Device *device) {
  check(device != NULL, "rtInitDevice(): device ptr passed is NULL");

  DeviceInt  *internal_device = (DeviceInt *)device;
  internal_device->state.values[STATE_WINDING] = RAST_WINDING_ORDER_CCW;
  internal_device->state.values[STATE_CULL_MODE] = RAST_CULL_MODE_BACK;
  internal_device->vbuff = NULL;
  internal_device->ibuff = NULL;
  internal_device->target = NULL;
  kmMat4Identity(&internal_device->model_mat);
  kmMat4Identity(&internal_device->projection_mat);
  kmMat4Identity(&internal_device->view_mat);

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

  internal_device->state.values[STATE_WINDING] = RAST_WINDING_ORDER_CCW;
  internal_device->state.values[STATE_CULL_MODE] = RAST_CULL_MODE_BACK;
  internal_device->vbuff = NULL;
  internal_device->ibuff = NULL;
  internal_device->target = NULL;
  kmMat4Identity(&internal_device->model_mat);
  kmMat4Identity(&internal_device->projection_mat);
  kmMat4Identity(&internal_device->view_mat);

  debug("rtClearDevice(): Cleared device");

  return 0;

error:

  return -1;
}
