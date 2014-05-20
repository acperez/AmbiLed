#ifndef ARDUINO_H
#define ARDUINO_H

#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#include "led_controller.h"

class Arduino: public LedController {
  public:
    Arduino(size_t aLedsLen, int aLedType);
    ~Arduino();

    int open(const char *aDevPath);
    int write(const uint8_t* aData, size_t aBytes);
    int read(uint8_t* aData, size_t aBytes);

  private:
    int initSerial(const char* aDevPath, int aSpeed);
    int writeInternal(const uint8_t* aData, size_t aBytes);

  private:
    int mFd;
    size_t mLedsLen;
    int mLedType;
};

#endif
