#ifndef CAPTURE_H
#define CAPTURE_H

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

#include "frame.h"

int initCaptureDevice(const char* device, int *width, int *height, int *format);
int captureFromDevice(Frame *frame);
void closeCaptureDevice();

#endif
