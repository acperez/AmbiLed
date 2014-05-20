#include "engine.h"

Engine::Engine(LedType aLedType, int aLedsUp, int aLedsRight,
               int aLedsDown, int aLedsLeft, size_t aWidth, size_t aHeight): mWidth(aWidth),
                                                                             mHeight(aHeight) {
  mLeds = new Leds(aLedType, aLedsUp, aLedsRight, aLedsDown, aLedsLeft, aWidth, aHeight);
}

Engine::~Engine() {
  delete mLeds;
}

int Engine::initLedController(const char *aDevPath, LedControllerType aControllerType) {
  if (!mLeds->initController(aDevPath, aControllerType))
    return 0;

  return mLeds->showInit();
}

RGBVal Engine::getAverage(const uint8_t *aData, Rect aMask) {
  unsigned int R = 0;
  unsigned int G = 0;
  unsigned int B = 0;
  size_t counter = 0;

  for (int x = aMask.x; x < aMask.x + aMask.width; x++)  {
    for (int y = aMask.y; y < aMask.y + aMask.height; y++) {
      int pixel = x * 3 + y * mWidth * 3;
      R += aData[pixel];
      G += aData[pixel + 1];
      B += aData[pixel + 2];
      counter++;
    }
  }

  return (RGBVal){ (uint8_t) (R / counter),
                   (uint8_t) (G / counter),
                   (uint8_t) (B / counter) };
}

int Engine::processFrame(const uint8_t *aData, size_t aBytes) {
  for (int i = 0; i < mLeds->getLength(); i++) {
    RGBVal color = getAverage(aData, mLeds->getMask(i));
    mLeds->setPixel(i, color);
  }

  return mLeds->show();
}
