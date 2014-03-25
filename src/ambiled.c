#include "capture.h"
#include "frame.h"
#include "common.h"

#define DEBUG 1

#ifdef DEBUG
#include <SDL2/SDL.h>
#endif

static int quit = 0;

int filterSDLQuitEvent(void *userdata, SDL_Event *event);
int initSDL(SDL_Window **window, SDL_Renderer **render, SDL_Texture **texture, int width, int height, int sdl_format);
int initParams(int v4l2_format, int width, int height, int *sdl_format, int *pitch, int *bytes);

int main(int argc, char **argv)
{
  // Input params

  // Default -> 640x480 || 720x576
  int width = 640;
  int height = 480;

  int v4l2_format = V4L2_PIX_FMT_RGB24; // V4L2_PIX_FMT_YUV420 || V4L2_PIX_FMT_YUYV || V4L2_PIX_FMT_RGB24 || -1

  SDL_Window *window = 0;
  SDL_Renderer *render = 0;
  SDL_Texture *texture = 0;
  Frame *frame = 0;
  int bytes, sdl_format, pitch;

  if (!initCaptureDevice("/dev/video0", &width, &height, &v4l2_format)) {
    exit(-1);
  }

  if (!initParams(v4l2_format, width, height, &sdl_format, &pitch, &bytes)) {
    closeCaptureDevice();
    error("Unknwn video color format\n");
    exit(-1);
  }

  if (!initSDL(&window, &render, &texture, width, height, sdl_format)) {
    closeCaptureDevice();
    exit(-1);
  }  

  frame = new_Frame(v4l2_format, bytes, width, height);
  SDL_UpdateTexture(texture, NULL, frame->data, pitch);

  SDL_SetEventFilter(filterSDLQuitEvent, NULL);
  while (!quit)
  {
    // Process events
    while (SDL_PollEvent(NULL));

    // Capture and process input
    if (captureFromDevice(frame)) {
      SDL_UpdateTexture(texture, NULL, frame->data, pitch);
      processFrame();
    }

    // Update render
    SDL_RenderClear(render);
    SDL_RenderCopy(render, texture, NULL, NULL);
    SDL_RenderPresent(render);
  }

  closeCaptureDevice();
  free_Frame(frame);
  SDL_DestroyRenderer(render);
  SDL_DestroyTexture(texture);
  SDL_Quit();

  return 0;
}

int filterSDLQuitEvent(void *userdata, SDL_Event *event) {
  if (event->type == SDL_QUIT) {
    quit = 1;
    return 0;
  }

  return 0;
}

int initParams(int v4l2_format, int width, int height, int *sdl_format, int *pitch, int *bytes) {
  switch (v4l2_format) {
    case V4L2_PIX_FMT_RGB24:  // RGB 8:8:8
      *sdl_format = SDL_PIXELFORMAT_RGB24;
      *bytes = width * height * 3;
      *pitch = width * 3;
      return 1;

    case V4L2_PIX_FMT_YUYV:   // YUV 4:2:2
      *sdl_format = SDL_PIXELFORMAT_YUY2;
      *bytes = width * height * 2;
      *pitch = width * 2;
      return 1;

    case V4L2_PIX_FMT_YUV420: // YUV 4:2:0
      *sdl_format = SDL_PIXELFORMAT_IYUV;
      *bytes = width * height * 3 / 2;
      *pitch = width;
      return 1;

    default:
      return 0;
  }
}

int initSDL(SDL_Window **window, SDL_Renderer **render, SDL_Texture **texture, int width, int height, int sdl_format) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    error("Could not initialize SDL: %s\n", SDL_GetError());
    return 0;
  }

  *window = SDL_CreateWindow("Input stream", 0, 0, width, height, SDL_WINDOW_SHOWN);
  if (!*window) {
    error("Could not create window: %s\n", SDL_GetError());
    return 0;
  }

  *render = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
  if (!*render) {
    error("Could not create render: %s\n", SDL_GetError());
    return 0;
  }

  *texture = SDL_CreateTexture(*render, sdl_format, SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!*texture) {
    error("Could not create texture: %s\n", SDL_GetError());
    return 0;
  }

  return 1;
}
