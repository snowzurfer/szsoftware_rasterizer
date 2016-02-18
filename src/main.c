
#include "thread_pool_c.h"
#include "rasterizer.h"
#include <stdio.h>

int main() {
  Device *device = rtCreateDevice();
  CmdBuffer *cmdbuf = rtCreateCmdBuffer(4096U);
  VertexBuffer vtxbuff;



  rtDestroyCmdBuffer(cmdbuf);
  rtDestroyDevice(device);

  getc(stdin);
  return 0;
};
