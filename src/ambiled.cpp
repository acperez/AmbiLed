//#include <SDL2/SDL.h>
#ifdef DEBUG
#include <time.h>
#endif

#include <libconfig.h>
#include <signal.h>

#include "usb_grabber.h"
#include "engine.h"
#include "leds.h"
#include "arduino.h"
#include "video.h"
#include "common.h"

static int quit = 0;

typedef struct {
  const char *filename;
  size_t width;
  size_t height;
  int ledsRight;
  int ledsUp;
  int ledsLeft;
  int ledsDown;
  const char *videoPath;
  const char *ledControllerPath;
  VideoType videoType;
  LedType ledType;
  LedControllerType ledControllerType;
} Config;

void readConf(const char *aFilename, Config *config);

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
  uint32_t v4l2_format = V4L2_PIX_FMT_RGB24;
  int bytes, pitch;
  Capturer *capturer;
  Engine *engine;
  uint8_t *frameBuffer;

  const char *config_filename = "ambiled.conf";

  Config config;
  readConf(config_filename, &config);

#ifdef DEBUG
  struct timeval stop, start;
#endif

  if(!setOverCommitMemory("1")) {
    exit(-1);
  }

//  SDL_Window *windowIn = 0;
//  SDL_Window *windowOut = 0;
//  SDL_Renderer *renderIn = 0;
//  SDL_Renderer *renderOut = 0;
//  SDL_Texture *textureIn = 0;
//  SDL_Texture *textureOut = 0;
//  int sdl_format;

  switch (config.videoType) {
    case CAPTURER:
      capturer = new USBGrabber();
      break;
    case PLAYER:
      capturer = new Video();
      break;
    case UNKNOWN:
      error("Unknown video type\n");
      exit(-1);
  }

  if(!capturer->open(config.videoPath, &config.width, &config.height, &v4l2_format)) {
    setOverCommitMemory("0");
    exit(-1);
  }

//  if (!initParams(v4l2_format, width, height, &sdl_format, &pitch, &bytes)) {
  if (!initParams(v4l2_format, config.width, config.height, &pitch, &bytes)) {
    delete capturer;
    error("Unknwn video color format\n");
    setOverCommitMemory("0");
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

  engine = new Engine(config.ledType, config.ledsUp, config.ledsRight, config.ledsDown, config.ledsLeft, config.width, config.height);
  if (!engine->initLedController(config.ledControllerPath, config.ledControllerType)) {
//  if (!engine->initLedController(piDev, controllerType)) {
    delete capturer;
    setOverCommitMemory("0");
    exit(-1);
  }

  frameBuffer = (uint8_t *) malloc(bytes);

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
    if (capturer->getFrame(frameBuffer, bytes)) {
//      SDL_UpdateTexture(textureIn, NULL, buffer, pitch);
      engine->processFrame(frameBuffer, bytes);
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
  free(frameBuffer);
//  SDL_DestroyRenderer(renderIn);
//  SDL_DestroyTexture(textureIn);
//  SDL_DestroyRenderer(renderOut);
//  SDL_DestroyTexture(textureOut);
//  SDL_Quit();
  setOverCommitMemory("0");

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

void confError(const char *filename, const char *param) {
  error("Missing param \'%s\' in config file %s\n", param, filename);
  exit(-1);
}

void confInvalid(const char *filename, const char *param, const char *value) {
  error("Invalid value \'%s\' for param \'%s\' in config file %s\n", value, param, filename);
  exit(-1);
}

void readConf(const char *aFilename, Config * config) {
  config_t cfg;
  config_init(&cfg);

  // Open config file
  if (!config_read_file(&cfg, aFilename)) {
    error("Can't open config file %s: %d - %s\n", aFilename, config_error_line(&cfg), config_error_text(&cfg));
    config_destroy(&cfg);
    exit(-1);
  }

  config->filename = aFilename;

  config->videoType = UNKNOWN;
  config->ledType = EMPTY;
  config->ledControllerType = NONE;

  // Load config groups
  config_setting_t * video = config_lookup(&cfg, "video");
  if (!video) {
    error("Missing video configuration in config file %s\n", aFilename);
    exit(-1);
  }

  config_setting_t * videoDevice = config_lookup(&cfg, "video/device");
  if (!videoDevice) {
    error("Missing video device configuration in config file %s\n", aFilename);
    exit(-1);
  }

  config_setting_t * leds = config_lookup(&cfg, "leds");
  if (!leds) {
    error("Missing leds configuration in config file %s\n", aFilename);
    exit(-1);
  }

  config_setting_t * ledController = config_lookup(&cfg, "leds/controller");
  if (!ledController) {
    error("Missing led controller configuration in config file %s\n", aFilename);
    exit(-1);
  }

  // Load params

  // Video width
  int width;
  if (!config_setting_lookup_int(video, "width", &width))
    confError(aFilename, "video/width");

  config->width = width;

  // Video height
  int height;
  if (!config_setting_lookup_int(video, "height", &height))
    confError(aFilename, "video/height");

  config->height = height;

  // Video device type
  const char *videoDeviceType;
  if (!config_setting_lookup_string(videoDevice, "type", &videoDeviceType))
    confError(aFilename, "video/device/type");

  if (strcmp(videoDeviceType, "CAPTURER") == 0) config->videoType = CAPTURER;
  if (strcmp(videoDeviceType, "PLAYER") == 0) config->videoType = PLAYER; 
  if (config->videoType == UNKNOWN) confInvalid(aFilename, "video/device/type", videoDeviceType);

  // Video device path
  if (!config_setting_lookup_string(videoDevice, "path", &config->videoPath))
    confError(aFilename, "video/device/path");

  // Led type
  const char *ledDeviceType;
  if (!config_setting_lookup_string(leds, "type", &ledDeviceType))
    confError(aFilename, "leds/type");

  if (strcmp(ledDeviceType, "WS2801") == 0) config->ledType = WS2801;
  if (strcmp(ledDeviceType, "WS2812B") == 0) config->ledType = WS2812B;
  if (strcmp(ledDeviceType, "TM1804") == 0) config->ledType = TM1804;
  if (config->ledType == EMPTY) confInvalid(aFilename, "leds/type", ledDeviceType);

  // Leds left
  if (!config_setting_lookup_int(leds, "left", &config->ledsLeft))
    confError(aFilename, "leds/left");

  // Leds up
  if (!config_setting_lookup_int(leds, "up", &config->ledsUp))
    confError(aFilename, "leds/up");

  // Leds right
  if (!config_setting_lookup_int(leds, "right", &config->ledsRight))
    confError(aFilename, "leds/right");

  // Leds down
  if (!config_setting_lookup_int(leds, "down", &config->ledsDown))
    confError(aFilename, "leds/down");

  // Led controller type
  const char *ledControllerDeviceType;
  if (!config_setting_lookup_string(ledController, "type", &ledControllerDeviceType))
    confError(aFilename, "leds/controller/type");

  if (strcmp(ledControllerDeviceType, "ARDUINO") == 0) config->ledControllerType = ARDUINO;
  if (strcmp(ledControllerDeviceType, "RASPBERRY") == 0) config->ledControllerType = RASPBERRY;
  if (config->ledControllerType == NONE) confInvalid(aFilename, "leds/controller/type", ledControllerDeviceType);

  // Led controller device pat
  if (!config_setting_lookup_string(ledController, "path", &config->ledControllerPath))
    confError(aFilename, "leds/controller/path");
}
