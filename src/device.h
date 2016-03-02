/* Private header file */

#ifndef DEVICE_H 
#define DEVICE_H

#include <assert.h>
#include "rasterizer.h"
#include "command_buffer.h"

#define STATE_COUNT 2
#define STATE_WINDING 0
#define STATE_CULL_MODE 1

typedef struct DeviceState {
  uint32_t values[STATE_COUNT];
} DeviceState;

typedef struct DeviceInt {
  DeviceState state;
  CmdBuffersQueue cmdbuffer_queue;
} DeviceInt;
static_assert(sizeof(Device) >= sizeof(DeviceInt),
  "Size of Device must be >= the size of DeviceInt");


#endif
