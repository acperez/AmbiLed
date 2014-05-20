#ifndef LEDS_H
#define LEDS_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "common.h"
#include "led_controller.h"
#include "arduino.h"
#include "gpio.h"

class Leds {
  public:
    Leds(LedType aLedType, int aLedsUp, int aLedsRight, int aLedsDown, int aLedsLeft, int aWidth, int aHeight);
    ~Leds();

    int initController(const char *aDevPath, LedControllerType aControllerType);
    int getLength();
    Rect getMask(int i);
    void setPixel(int aIndex, RGBVal aColor);
    int showInit();
    int show();

  private:
    Rect createMask(int aX, int aY, int aMaxWidth, int aMaxHeight);
    int showColor(RGBVal aColor);

  private:
    LedController *mController;
    LedType mLedType;
    int mLength;
    Rect *mMasks;
    uint8_t **mGamma;
    uint8_t *mBuffer;
    size_t mBytes;

};

#endif
