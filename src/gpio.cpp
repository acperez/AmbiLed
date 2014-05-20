#include "gpio.h"

Gpio::Gpio(): mFd(0) {
}

Gpio::~Gpio(){
  close(mFd);
}

int Gpio::open(const char *aDevPath) {
  if (mFd != 0) {
    error("Raspberry already initialized\n");
    return 0;
  }

  mFd = ::open(aDevPath, O_WRONLY);
  if (mFd == -1) {
    error("Error %d opening %s: %s\n", errno, aDevPath, strerror (errno));
    return 0;
  }

  if (!initSPI()) {
    close(mFd);
    mFd = 0;
    return 0;
  }

  return 1;
}

int Gpio::initSPI() {
  uint8_t mode = SPI_MODE_0;
  uint8_t bits = 8;
  uint8_t lsbf = 0;
  uint32_t speed = 1000000;

  // Set SPI mode
  if (ioctl(mFd, SPI_IOC_WR_MODE, &mode) < 0) {
    error("Can't set spi mode: %d - %s\n", errno, strerror(errno));
    return 0;
  }

  // Set bits per word
  if (ioctl(mFd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
    error("Can't set bits per word: %d - %s\n", errno, strerror(errno));
    return 0;
  }

  // Set max speed
  if (ioctl(mFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
    error("Can't set max speed: %d - %s\n", errno, strerror(errno));
    return 0;
  }

  if (ioctl(mFd, SPI_IOC_WR_LSB_FIRST, &lsbf) < 0) {
    error("Can't set bit order: %d - %s", errno, strerror(errno));
  }

  return 1;
}

int Gpio::write(const uint8_t *aData, size_t aBytes) {
  uint8_t rx[aBytes];
  uint16_t delay = 0;
  struct spi_ioc_transfer mesg;

  mesg.tx_buf        = (unsigned long) aData;
  mesg.rx_buf        = (unsigned long) rx;
  mesg.len           = aBytes;
  mesg.delay_usecs   = delay;
  mesg.speed_hz      = 1000000;
  mesg.bits_per_word = 8;

  if (ioctl(mFd, SPI_IOC_MESSAGE(1), &mesg) < 0) {
    error("Couldn't write whole data to SPI: %d - %s\n", errno, strerror (errno));
    return 0;
  }

  return 1;
}

int Gpio::read(uint8_t *aData, size_t aBytes) {
  error("Not implemented");
  return 0;
}
