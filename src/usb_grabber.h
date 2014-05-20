#ifndef USBGRABBER_H
#define USBGRABBER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

#include "capturer.h"
#include "common.h"

struct buffer
{
  void   *start;
  size_t length;
};

class USBGrabber: public Capturer {
  public:
    USBGrabber();
    ~USBGrabber();

    int open(const char* aDevPath, size_t *aWidth, size_t *aHeight, uint32_t *aFormat);
    int getFrame(uint8_t *aData, size_t aBytes);

  private:
    int validateCaptureDevice();
    int initCaptureFormat();
    int setCaptureFormat(size_t *aWidth, size_t *aHeight, uint32_t *aFormat);
    int getCaptureInfo();
    int initBuffers();
    int startStreaming();

  private:
    int mFd;
    struct v4l2_format mFmt;
    struct buffer *mBuf;
    size_t mBuffers;
    enum v4l2_buf_type mType;
};

#endif
