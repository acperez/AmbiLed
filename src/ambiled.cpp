//#include <SDL2/SDL.h>
#ifdef DEBUG
#include <time.h>
#endif

#include "usb_grabber.h"
#include "engine.h"
#include "leds.h"
#include "arduino.h"
#include "video.h"
#include "common.h"
#include <signal.h>

static int quit = 0;

//int filterSDLQuitEvent(void *userdata, SDL_Event *event);
//int initSDL(SDL_Window **window, SDL_Renderer **render, SDL_Texture **texture,
//            int posX, int posY, int width, int height, int sdl_format, const char *title);
//int initParams(int v4l2_format, int width, int height, int *sdl_format, int *pitch, int *bytes);

int initParams(int v4l2_format, int width, int height, int *pitch, int *bytes);
int setOverCommitMemory(const char* value);

void intHandler(int sign) {
  quit = 1;
}

int main(int argc, char **argv)
{
  // Input params
  size_t width = 320;
  size_t height = 240;
//  size_t width = 640;
//  size_t height = 480;

  int leds_up = 24;
  int leds_right = 13;
  int leds_down = 0;
  int leds_left = 13;

  const char *arduinoDev = "/dev/ttyACM0";
//  const char* piDev = "/dev/spidev0.0";

  LedControllerType controllerType = ARDUINO;
//  LedControllerType controllerType = RASPBERRY;
  LedType ledType = WS2801;

#ifdef DEBUG
  struct timeval stop, start;
#endif

  uint32_t v4l2_format = V4L2_PIX_FMT_RGB24;

////  if(!setOverCommitMemory("1")) {
////    exit(-1);
////  }

//  SDL_Window *windowIn = 0;
//  SDL_Window *windowOut = 0;
//  SDL_Renderer *renderIn = 0;
//  SDL_Renderer *renderOut = 0;
//  SDL_Texture *textureIn = 0;
//  SDL_Texture *textureOut = 0;
  int bytes, pitch;
//  int sdl_format;

  Capturer *capturer = new USBGrabber();
  if(!capturer->open("/dev/video1", &width, &height, &v4l2_format)) {
////    setOverCommitMemory("0");
    exit(-1);
  }

//  Capturer *capturer = new Video();
//  if (!capturer->open("/home/acperez/ssd/ambilight/the-lion-king-bluray-trailer-720p-HDTN.mp4", &width, &height, &v4l2_format)) {
//    exit(-1);
//  }

//  if (!initParams(v4l2_format, width, height, &sdl_format, &pitch, &bytes)) {
  if (!initParams(v4l2_format, width, height, &pitch, &bytes)) {
    delete capturer;
    error("Unknwn video color format\n");
////    setOverCommitMemory("0");
    exit(-1);
  }

//  if (!initSDL(&windowIn, &renderIn, &textureIn, 0, 0, width, height, sdl_format, "Input stream")) {
//    delete capturer;
//    exit(-1);
//  }

//  if (!initSDL(&windowOut, &renderOut, &textureOut, width + 2, 0, width, height, sdl_format, "Leds")) {
//    delete capturer;
//    exit(-1);
//  }

  Engine *engine = new Engine(ledType, leds_up, leds_right, leds_down, leds_left, width, height);
  if (!engine->initLedController(arduinoDev, controllerType)) {
//  if (!engine->initLedController(piDev, controllerType)) {
    delete capturer;
////    setOverCommitMemory("0");
    exit(-1);
  }

  uint8_t *buffer = (uint8_t *) malloc(bytes);

//  SDL_UpdateTexture(textureIn, NULL, buffer, pitch);
//  SDL_UpdateTexture(textureOut, NULL, buffer, pitch);

//  SDL_SetEventFilter(filterSDLQuitEvent, NULL);

  signal(SIGINT, intHandler);

  while (!quit) {

#ifdef DEBUG
  gettimeofday(&start, NULL);
#endif

//    // Process events
//    while (SDL_PollEvent(NULL));

    // Capture and process input
    if (capturer->getFrame(buffer, bytes)) {
//      SDL_UpdateTexture(textureIn, NULL, buffer, pitch);
      engine->processFrame(buffer, bytes);
//      SDL_UpdateTexture(textureOut, NULL, output, pitch);
    }

//    // Update render
//    SDL_RenderClear(renderIn);
//    SDL_RenderCopy(renderIn, textureIn, NULL, NULL);
//    SDL_RenderPresent(renderIn);
//    SDL_RenderClear(renderOut);
//    SDL_RenderCopy(renderOut, textureOut, NULL, NULL);
//    SDL_RenderPresent(renderOut);

//#ifdef DEBUG
//  gettimeofday(&stop, NULL);
//  long time = stop.tv_usec - start.tv_usec;
//  printf("took %lu\n", (stop.tv_usec - start.tv_usec) / 1000);
//
//  long freeTime = (41.6 - (time / 1000)) * 1000 * 1000;
//  if (freeTime > 0) {
//    print("---------- Sleep %lu\n", freeTime / (1000 * 1000));
//    nanosleep((struct timespec[]){{0, freeTime}}, NULL);
//  }
//
//#endif
  }

  delete capturer;
  delete engine;
//  SDL_DestroyRenderer(renderIn);
//  SDL_DestroyTexture(textureIn);
//  SDL_DestroyRenderer(renderOut);
//  SDL_DestroyTexture(textureOut);
//  SDL_Quit();
////  setOverCommitMemory("0");

  return 0;
}

//int filterSDLQuitEvent(void *userdata, SDL_Event *event) {
//  if (event->type == SDL_QUIT || event->window.event == SDL_WINDOWEVENT_CLOSE) {
//    quit = 1;
//    return 0;
//  }
//
//  return 0;
//}

//int initParams(int v4l2_format, int width, int height, int *sdl_format, int *pitch, int *bytes) {
//  switch (v4l2_format) {
//    case V4L2_PIX_FMT_RGB24:  // RGB 8:8:8
//      *sdl_format = SDL_PIXELFORMAT_RGB24;
//      *bytes = width * height * 3;
//      *pitch = width * 3;
//      return 1;
//
//    case V4L2_PIX_FMT_YUYV:   // YUV 4:2:2
//      *sdl_format = SDL_PIXELFORMAT_YUY2;
//      *bytes = width * height * 2;
//      *pitch = width * 2;
//      return 1;
//
//    case V4L2_PIX_FMT_YUV420: // YUV 4:2:0
//      *sdl_format = SDL_PIXELFORMAT_IYUV;
//      *bytes = width * height * 3 / 2;
//      *pitch = width;
//      return 1;
//
//    default:
//      return 0;
//  }
//}

int initParams(int v4l2_format, int width, int height, int *pitch, int *bytes) {
  switch (v4l2_format) {
    case V4L2_PIX_FMT_RGB24:  // RGB 8:8:8
      *bytes = width * height * 3;
      *pitch = width * 3;
      return 1;

    case V4L2_PIX_FMT_YUYV:   // YUV 4:2:2
      *bytes = width * height * 2;
      *pitch = width * 2;
      return 1;

    case V4L2_PIX_FMT_YUV420: // YUV 4:2:0
      *bytes = width * height * 3 / 2;
      *pitch = width;
      return 1;

    default:
      return 0;
  }
}

int setOverCommitMemory(const char* value) {
  int fd;
  if ((fd=open("/proc/sys/vm/overcommit_memory", O_RDWR)) < 0) {
    error("Can't set overcommit memory param: %s\n", strerror(errno));
    return 0;
  }

  if (write (fd, value, strlen(value)) < 0) {
    error("Can't set overcommit memory param: %s\n", strerror(errno));
    return 0;
  }

  close(fd);
  return 1;
}

//int initSDL(SDL_Window **window, SDL_Renderer **render, SDL_Texture **texture,
//            int posX, int posY, int width, int height, int sdl_format, const char *title) {
//  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
//    error("Could not initialize SDL: %s\n", SDL_GetError());
//    return 0;
//  }
//
//  *window = SDL_CreateWindow(title, posX, posY, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
//  *window = SDL_CreateWindow(title, posX, posY, width, height, SDL_WINDOW_SHOWN);
//  if (!*window) {
//    error("Could not create window: %s\n", SDL_GetError());
//    return 0;
//  }
//
//  *render = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
//  if (!*render) {
//    error("Could not create render: %s\n", SDL_GetError());
//    return 0;
//  }
//
//  *texture = SDL_CreateTexture(*render, sdl_format, SDL_TEXTUREACCESS_STREAMING, width, height);
//  if (!*texture) {
//    error("Could not create texture: %s\n", SDL_GetError());
//    return 0;
//  }
//
//  return 1;
//}
