
//#include "thread_pool.h"
//#include <iostream>
//#include <thread>
//#include <atomic>
//#include <pthread.h>
//#include <Windows.h>
#include "dbg.h"
#include "thread_pool_c.h"
#include "rasterizer.h"
#include <stdio.h>

int main() {
  
  Device *device = NULL;
  device = rtCreateDevice();

  rtDestroyDevice(device);

  getc(stdin);
  return 0;
};
