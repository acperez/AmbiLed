#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "common.h"

class LedController {
  public:
    LedController() {};
    virtual ~LedController() {};

    virtual int open(const char *aDevPath) = 0;
    virtual int write(const uint8_t* aData, size_t aBytes) = 0;
    virtual int read(uint8_t* aData, size_t aBytes) = 0;
};

#endif
