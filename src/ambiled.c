#include "capture.h"
#include "frame.h"
#include "leds.h"
#include "common.h"

#define DEBUG 1

#ifdef DEBUG
#include <SDL2/SDL.h>
#endif

static int quit = 0;

int filterSDLQuitEvent(void *userdata, SDL_Event *event);
int initSDL(SDL_Window **window, SDL_Renderer **render, SDL_Texture **texture,
            int posX, int posY, int width, int height, int sdl_format, const char *title);
int initParams(int v4l2_format, int width, int height, int *sdl_format, int *pitch, int *bytes);

int main(int argc, char **argv)
{
  // Input params

  // Default -> 640x480 || 720x576
  int width = 640;
  int height = 480;

  int leds_x = 30;
  int leds_y = 14;

  int v4l2_format = V4L2_PIX_FMT_RGB24; // V4L2_PIX_FMT_YUV420 || V4L2_PIX_FMT_YUYV || V4L2_PIX_FMT_RGB24 || -1

  SDL_Window *windowIn = 0;
  SDL_Window *windowOut = 0;
  SDL_Renderer *renderIn = 0;
  SDL_Renderer *renderOut = 0;
  SDL_Texture *textureIn = 0;
  SDL_Texture *textureOut = 0;
  Frame *frame = 0;
  Leds *leds = 0;
  int bytes, sdl_format, pitch;

  if (!initCaptureDevice("/dev/video0", &width, &height, &v4l2_format)) {
    exit(-1);
  }

  if (!initParams(v4l2_format, width, height, &sdl_format, &pitch, &bytes)) {
    closeCaptureDevice();
    error("Unknwn video color format\n");
    exit(-1);
  }

  if (!initSDL(&windowIn, &renderIn, &textureIn, 0, 0, width, height, sdl_format, "Input stream")) {
    closeCaptureDevice();
    exit(-1);
  }

  if (!initSDL(&windowOut, &renderOut, &textureOut, width + 2, 0, width, height, sdl_format, "Leds")) {
    closeCaptureDevice();
    exit(-1);
  }

  leds = initLeds(leds_x, leds_y, width, height);

  frame = new_Frame(v4l2_format, bytes, width, height);
  SDL_UpdateTexture(textureIn, NULL, frame->data, pitch);
  SDL_UpdateTexture(textureOut, NULL, frame->data, pitch);

  SDL_SetEventFilter(filterSDLQuitEvent, NULL);
  while (!quit)
  {
    // Process events
    while (SDL_PollEvent(NULL));

    // Capture and process input
    if (captureFromDevice(frame)) {
      SDL_UpdateTexture(textureIn, NULL, frame->data, pitch);
      unsigned char *output = processFrame(frame, leds);
      SDL_UpdateTexture(textureOut, NULL, output, pitch);
      free(output);
    }

    // Update render
    SDL_RenderClear(renderIn);
    SDL_RenderCopy(renderIn, textureIn, NULL, NULL);
    SDL_RenderPresent(renderIn);
    SDL_RenderClear(renderOut);
    SDL_RenderCopy(renderOut, textureOut, NULL, NULL);
    SDL_RenderPresent(renderOut);
  }

  closeCaptureDevice();
  free_Frame(frame);
  freeLeds(leds);
  SDL_DestroyRenderer(renderIn);
  SDL_DestroyTexture(textureIn);
  SDL_DestroyRenderer(renderOut);
  SDL_DestroyTexture(textureOut);
  SDL_Quit();

  return 0;
}

int filterSDLQuitEvent(void *userdata, SDL_Event *event) {
  if (event->type == SDL_QUIT || event->window.event == SDL_WINDOWEVENT_CLOSE) {
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

int initSDL(SDL_Window **window, SDL_Renderer **render, SDL_Texture **texture,
            int posX, int posY, int width, int height, int sdl_format, const char *title) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    error("Could not initialize SDL: %s\n", SDL_GetError());
    return 0;
  }

  *window = SDL_CreateWindow(title, posX, posY, width, height, SDL_WINDOW_SHOWN);
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
