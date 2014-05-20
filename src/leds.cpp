#include "leds.h"

#define MASK_WIDTH 20
#define MASK_HEIGHT 10

Leds::Leds(LedType aLedType, int aLedsUp, int aLedsRight,
           int aLedsDown, int aLedsLeft, int aWidth, int aHeight): mLedType(aLedType) {
  mLength = aLedsUp + aLedsRight + aLedsDown + aLedsLeft;

  mBytes = mLength * 3;
  mBuffer = (uint8_t*) malloc((mBytes) * sizeof(uint8_t));
  memset(mBuffer, 0, mBytes);

  mMasks = (Rect *) malloc((mLength) * sizeof(Rect));
  int index = 0;

  // Vertical leds left / stream pixels mapping
  int distance = aHeight / (aLedsLeft - 1);
  int offset = (aHeight - (distance * (aLedsLeft - 1))) / 2;
  for (int i = 0; i < aLedsLeft; i++) {
    int y = aHeight - ((i * distance) + offset);
    mMasks[index] = createMask(0, y, aWidth, aHeight);
    index++;
  }

  // Horizontal leds up / stream pixels mapping
  distance = aWidth / (aLedsUp - 1);
  offset = (aWidth - (distance * (aLedsUp - 1))) / 2;
  for (int i = 0; i < aLedsUp; i++) {
    int x = (i * distance) + offset;
    mMasks[index] = createMask(x, 0, aWidth, aHeight);
    index++;
  }

  // Vertical leds right / stream pixels mapping
  distance = aHeight / (aLedsRight - 1);
  offset = (aHeight - (distance * (aLedsRight - 1))) / 2;
  for (int i = 0; i < aLedsRight; i++) {
    int y = (i * distance) + offset;
    mMasks[index] = createMask(aWidth - 1, y, aWidth, aHeight);
    index++;
  }

  // Horizontal leds down / stream pixels mapping
  distance = aWidth / (aLedsDown - 1);
  offset = (aWidth - (distance * (aLedsDown - 1))) / 2;
  for (int i = 0; i < aLedsDown; i++) {
    int x = aWidth - ((i * distance) + offset);
    mMasks[index] = createMask(x, aHeight - 1, aWidth, aHeight);
    index++;
  }

  float f;
  mGamma = (uint8_t**)malloc(256 * sizeof(uint8_t*));
  for (int i = 0; i < 256; i++) {
    f           = pow((float) i / 255.0, 2.8);
    mGamma[i]    = (uint8_t*)malloc(3 * sizeof(uint8_t*));
    mGamma[i][0] = (uint8_t)(f * 255.0);
    mGamma[i][1] = (uint8_t)(f * 240.0);
    mGamma[i][2] = (uint8_t)(f * 220.0);
  }
}

Leds::~Leds() {
  showColor(RGBVal(0, 0, 0));

  delete mController;
  free(mBuffer);
  free(mMasks);

  for (int i = 0; i < 256; i++) {
    free(mGamma[i]);
  }
  free(mGamma);
}

int Leds::initController(const char *aDevPath, LedControllerType aControllerType) {
  switch (aControllerType) {
    case ARDUINO:
      mController = new Arduino(mLength, mLedType);
    break;
    case RASPBERRY:
      mController = new Gpio();
    break;
    default:
      error("Unknown led controller");
      return 0;
  }

  if (!mController->open(aDevPath))
    return 0;

  return 1;
}

Rect Leds::createMask(int aX, int aY, int aMaxWidth, int aMaxHeight) {
  int rectangleX = MASK_WIDTH;
  int rectangleY = MASK_HEIGHT;

  int x = aX - (rectangleX / 2);
  int y = aY - (rectangleY / 2);

  int offsetX = 0;
  int offsetY = 0;

  if (x < 0)
    offsetX = x - (2 * x);

  if (x + rectangleX > aMaxWidth - 1)
    offsetX = aMaxWidth - (x + rectangleX);

  if (y < 0)
    offsetY = y - (2 * y);

  if (y + rectangleY > aMaxHeight - 1)
    offsetY = aMaxHeight - (y + rectangleY);

  return (Rect){x + offsetX, y + offsetY, rectangleX, rectangleY};
}

int Leds::getLength() {
  return mLength;
}

Rect Leds::getMask(int i) {
  return mMasks[i];
}

int Leds::showInit() {
  if (!showColor(RGBVal(255, 0, 0))) {
    return 0;
  }

  usleep(500 * 1000);

  if (!showColor(RGBVal(0, 255, 0))) {
    return 0;
  }

  usleep(500 * 1000);

  if (!showColor(RGBVal(0, 0, 255))) {
    return 0;
  }

  usleep(500 * 1000);

  if (!showColor(RGBVal(0, 0, 0))) {
    return 0;
  }

  usleep(500 * 1000);
  return 1;
}

int Leds::showColor(RGBVal aColor) {
  for (int index = 0; index < mLength; index++) {
    mBuffer[index * 3]     = aColor.R;
    mBuffer[index * 3 + 1] = aColor.G;
    mBuffer[index * 3 + 2] = aColor.B;
  }

  if (!mController->write(mBuffer, mBytes)) {
    return 0;
  }

  return 1;
}

void Leds::setPixel(int aIndex, RGBVal aColor) {
  /*nt minBrightness = 120;
  // Boost pixels that fall below the minimum brightness
  int sum = r + g + b;
  int offset;
  if(sum < minBrightness) {
    if(sum == 0) {
      offset = minBrightness / 3; // Spread equally to R,G,B
      r += offset;
      g += offset;
      b += offset;
    } else {
      offset = minBrightness - sum;
      int s2 = sum * 2;
      r += offset * (sum - r) / s2;
      g += offset * (sum - g) / s2;
      b += offset * (sum - b) / s2;
    }
  }*/

  // Apply gamma correction
  mBuffer[aIndex * 3]     = mGamma[aColor.R][0];
  mBuffer[aIndex * 3 + 1] = mGamma[aColor.G][1];
  mBuffer[aIndex * 3 + 2] = mGamma[aColor.B][2];
}

int Leds::show() {
  if (!mController->write(mBuffer, mBytes)) {
    return 0;
  }

  return 1;
}
