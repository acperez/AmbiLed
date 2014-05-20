#ifndef CAPTURER_H
#define CAPTURER_H

#include "common.h"

class Capturer {
  public:
    Capturer() {};
    virtual ~Capturer() {};

    virtual int open(const char* aFilePath, size_t *aWidth, size_t *aHeight, uint32_t *aFormat) = 0;
    virtual int getFrame(uint8_t *aData, size_t aBytes) = 0;
};

#endif
