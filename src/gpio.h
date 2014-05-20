#ifndef GPIO_H
#define GPIO_H

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <errno.h>
#include <string.h>

#include "led_controller.h"

class Gpio: public LedController {
  public:
    Gpio();
    ~Gpio();

    int open(const char *aDevPath);
    int write(const uint8_t* aData, size_t bytes);
    int read(uint8_t* aData, size_t bytes);

  private:
    int initSPI();

  private:
    int mFd;
};

#endif
