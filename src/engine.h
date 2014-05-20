#ifndef ENGINE_H
#define ENGINE_H

#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "leds.h"

class Engine {
  public:
    Engine(LedType aLedType, int aLedsUp, int aLedsRight, int aLedsDown, int aLedsLeft, size_t aWidth, size_t aHeight);
    ~Engine();

    int initLedController(const char *aDevPath, LedControllerType aControllerType);
    int processFrame(const uint8_t *aData, size_t aBytes);

  private:
    RGBVal getAverage(const uint8_t *aData, Rect aMask);

  private:
    Leds *mLeds;
    size_t mWidth;
    size_t mHeight;
};

#endif
