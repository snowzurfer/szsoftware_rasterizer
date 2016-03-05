
#include "thread_pool_c.h"
#include "rasterizer.h"
#include <stdio.h>
#include <stdlib.h>
#include "dbg.h"
#include "mat4.h"
#include "vec3.h"
#include "SDL.h"

#define FALSE 0
#define SCREEN_WIDTH 800U
#define SCREEN_HEIGHT 600U

/*Local function prototypes*/
static int32_t initSDL();
static void closeSDL();

/*File global vars*/
static SDL_Window *sdl_window = NULL;
static SDL_Event sdl_event;
static SDL_Texture *sdl_texture = NULL;
static SDL_Renderer *sdl_renderer = NULL;
static uint32_t run = 1U;
static uint32_t curr_ticks = 0U;
static uint32_t last_ticks = 0U;
static uint32_t delta_ticks = 0U;

int32_t main() {
  int32_t rc = -1;

  rc = initSDL();
  check(rc >= 0, "Failed to init SDL");

  Device device;
  rc = rtInitDevice(&device);
  check(rc != -1, "Device couldn't be initialised");

  CmdBuffer cmdbuff;
  /* The data will be used by the CmdBuffer structure to store the commands */
  uint8_t cmdbuff_data[4096U]; 
  rc = rtInitCmdBuffer(&cmdbuff, &cmdbuff_data, 4096U);
  check(rc != -1, "Cmdbuffer couldn't be initialised");

  uint32_t num_vertices = 3U;
  uint32_t vertex_size = sizeof(float) * 3U;
  float *vertices = (float *)malloc(vertex_size * num_vertices);
  vertices[0U] = -0.8f; vertices[1U] = -0.8f; vertices[2U] = -1.f;
  vertices[3U] = 0.f; vertices[4U] = 0.8f; vertices[5U] = -1.f;
  vertices[6U] = 0.8f; vertices[7U] = -0.8f; vertices[8U] = -1.f;

  VertexBuffer vtxbuff;
  rc = rtInitVertexBuffer(&vtxbuff, vertices,
    vertex_size * num_vertices, vertex_size);
  check(rc != -1, "VBuffer couldn't be initialised");

  uint32_t num_indices = 3U;
  uint32_t *indices = (uint32_t *)malloc(sizeof(uint32_t) * num_indices);
  for (uint32_t i = 0U; i < num_indices; i++) {
    indices[i] = i;
  }

  IndexBuffer indbuff;
  rc = rtInitIndexBuffer(&indbuff, indices, num_indices * sizeof(uint32_t));
  check(rc != -1, "IBuffer couldn't be initialised");

  uint32_t target_w = 800U;
  uint32_t target_h = 600U;
  RenderTarget target;
  uint8_t *target_data =
    (uint8_t *)malloc(sizeof(uint32_t) * target_w * target_h);
  rc = rtInitRenderTarget(&target, target_data, target_w, target_h);
  check(rc != -1, "Target couldn't be initialised");
  RenderTarget target_back;
  uint8_t *target_back_data =
    (uint8_t *)malloc(sizeof(uint32_t) * target_w * target_h);
  rc = rtInitRenderTarget(&target_back, target_back_data, target_w, target_h);
  check(rc != -1, "Target back couldn't be initialised");

  kmMat4 model_mat, view_mat, proj_mat;
  kmVec3 cam_pos, cam_lookat, cam_up;
  kmVec3Fill(&cam_pos, 0.f, 0.f, 5.f);
  kmVec3Fill(&cam_lookat, 0.f, 0.f, -1.f);
  kmVec3Fill(&cam_up, 0.f, 1.f, 0.f);
  kmMat4RotationZ(&model_mat, kmPI / 10.f);
  kmMat4LookAt(&view_mat, &cam_pos, &cam_lookat, &cam_up);
  kmMat4PerspectiveProjection(&proj_mat, kmPI / 4.f, (float)target_w / target_h,
                              0.3f, 100.f);

  rtSetRenderTarget(&cmdbuff, &target);
  rtSetVertexBuffer(&cmdbuff, &vtxbuff);
  rtSetModelMat(&cmdbuff, (const float *)&model_mat);
  rtSetViewMat(&cmdbuff, (const float *)&view_mat);
  rtSetProjectionMat(&cmdbuff, (const float *)&proj_mat);
  rtSetIndexBuffer(&cmdbuff, &indbuff);
  rtSetWindingOrder(&cmdbuff, RAST_WINDING_ORDER_CCW);
  rtSetCullMode(&cmdbuff, RAST_CULL_MODE_BACK);
  rtDrawAuto(&cmdbuff, num_vertices);

  rtSubmit(&device, &cmdbuff);

  rtParseCmdBuffers(&device);

  last_ticks = SDL_GetTicks();
  while(run) {
    curr_ticks = SDL_GetTicks();
    delta_ticks = curr_ticks - last_ticks; 
    debug("FPS: %f\n", 1000.f / (float)delta_ticks);
    
    /*Poll events*/
    while(SDL_PollEvent(&sdl_event) != 0) {
      if(sdl_event.type == SDL_QUIT) {
        run = FALSE;
      }
    }

    kmMat4RotationZ(&model_mat, (kmPI / 10.f) * (float)(curr_ticks / 100U));
    rtSetModelMat(&cmdbuff, (const float *)&model_mat);
    rtClearRenderTarget(&cmdbuff, &target);
    rtDrawAuto(&cmdbuff, num_vertices);
    rtSubmit(&device, &cmdbuff);
    rtParseCmdBuffers(&device);

    SDL_RenderClear(sdl_renderer);
    SDL_UpdateTexture(
        sdl_texture,
        NULL,
        (const void *)target_data,
        sizeof(uint32_t) * target_w);
    SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);

    last_ticks = curr_ticks;
  }

  /*Render to ppm to test out rendering*/
  /*uint32_t u32_size = sizeof(uint32_t);*/
  /*FILE *of = fopen("render.ppm", "wb");*/
  /*fprintf(of, "P6\n%d %d\n255\n", target_w, target_h);*/
  /*for(uint32_t j = 0U; j < target_h; j++) {*/
    /*for(uint32_t i = 0U; i < target_w; i++) {*/
      /*static uint8_t colour[3U];*/
      /*colour[0U] = target_data[u32_size * (j * target_w + i)];*/
      /*colour[1U] = target_data[u32_size * (j * target_w + i) + 1U];*/
      /*colour[2U] = target_data[u32_size * (j * target_w + i) + 2U];*/
      /*fwrite(colour, 1U, 3U, of);*/
    /*}*/
  /*}*/

  /*fclose(of);*/
  
  rtResetRenderTarget(&target);
  free(target_data);
  free(indices);
  free(vertices);
  rtResetIndexBuffer(&indbuff);
  /*for (uint32_t i = 0U; i < 3U * num_vertices; i++) {*/
    /*vertices[i] = (float)i;*/
  /*}*/
  rtResetVertexBuffer(&vtxbuff);
  rtResetCmdBuffer(&cmdbuff);
  rtResetDevice(&device);

  /*getc(stdin);*/
 
  closeSDL(); 
  return 0;

error:

  closeSDL();
  return -1;
};

int32_t initSDL() {
  int32_t rc = 0;

  rc = SDL_Init(SDL_INIT_VIDEO);
  check(rc >= 0, "initSDL(): could not initialise SDL Video! SDL Error: %s",
        SDL_GetError());

  sdl_window = SDL_CreateWindow(
      "szrasterizer",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      SCREEN_WIDTH,
      SCREEN_HEIGHT,
      SDL_WINDOW_SHOWN);
  check(sdl_window != NULL, "initSDL(): couldn't create window! SDL Error: %s",
        SDL_GetError());
  
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  check(sdl_renderer != NULL, "initSDL(): couldn't create window! \
        SDL Error: %s", SDL_GetError());

  sdl_texture = SDL_CreateTexture(
      sdl_renderer,
      SDL_PIXELFORMAT_RGBA8888,
      SDL_TEXTUREACCESS_STREAMING,
      SCREEN_WIDTH,
      SCREEN_HEIGHT);
  check(sdl_texture != NULL, "initSDL(): couldn't create texture! \
        SDL Error: %s", SDL_GetError());

  return 0;

error:

  return -1;
}

void closeSDL() {
  SDL_DestroyTexture(sdl_texture);
  sdl_texture = NULL;

  SDL_DestroyRenderer(sdl_renderer);
  sdl_renderer = NULL;

  SDL_DestroyWindow(sdl_window);
  sdl_window = NULL;

  SDL_Quit();
}
