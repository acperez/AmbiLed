#include "usb_grabber.h"

USBGrabber::USBGrabber() {
  mFd       = 0;
  mBuf      = 0;
  mBuffers  = 0;
  mType     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  clear(mFmt);
  mFmt.type = mType;
}

USBGrabber::~USBGrabber() {
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (v4l2_ioctl(mFd, VIDIOC_STREAMOFF, &type) < 0 ) {
    error("Can't stop streaming I/O\n");
  }

  for (size_t i = 0; i < mBuffers; i++)
  {
    v4l2_munmap(mBuf[i].start, mBuf[i].length);
  }

  free(mBuf);
  v4l2_close(mFd);
  mFd = 0;
}

int USBGrabber::open(const char* aDevPath, size_t *aWidth, size_t *aHeight, uint32_t *aFormat) {
  if (mFd != 0) {
    error("Device already opened");
    return 0;
  }

  // Open capture device
  mFd = v4l2_open(aDevPath, O_RDWR | O_NONBLOCK, 0);
  if (mFd < 0)
  {
    error("Cannot open capturer device\n");
    return 0;
  }

  if (!validateCaptureDevice()) return 0;
  if (!initCaptureFormat()) return 0;
  if (!setCaptureFormat(aWidth, aHeight, aFormat)) return 0;
  if (!getCaptureInfo()) return 0;
  if (!initBuffers()) return 0;
  if (!startStreaming()) return 0;

  return 1;
}

int USBGrabber::getFrame(uint8_t *aData, size_t aBytes) {
  struct v4l2_buffer buf;
  fd_set         fds;
  struct timeval tv;
  int            r;

  do {
    FD_ZERO(&fds);
    FD_SET(mFd, &fds);

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    r = select(mFd + 1, &fds, NULL, NULL, &tv);
  } while ((r == -1 && (errno = EINTR)));

  if (r == -1) {
    error("select error %d, %s\n", errno, strerror(errno));
    return -1;
  }

  clear(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  // Enqueue an empty (capturing) buffer in the driver's incoming queue.
  if (v4l2_ioctl(mFd, VIDIOC_DQBUF, &buf) < 0 ) {
    error("Can't queue buffer\n");
    return 0;
  }

  if (buf.index >= mBuffers) {
    error("Invalid buffer index received\n");
    return 0;
  }

  if (buf.bytesused != aBytes) {
    error("Capturing format mismatch\n");
    return 0;
  }

  memcpy(aData, mBuf[buf.index].start, buf.bytesused);

  // Dequeue a filled (capturing) buffer from the driver's outgoing queue.
  if (v4l2_ioctl(mFd, VIDIOC_QBUF, &buf) < 0 ) {
    error("Can't dequeue buffer\n");
    return 0;
  }

  return 1;
}

static void partstd2s(const char *stds[], unsigned long long std) {
  int first = 1;

  while (*stds) {
    if (std & 1) {
      if (!first)
        print("/");
      first = 0;
      print("%s", *stds);
    }
    stds++;
    std >>= 1;
  }

  print("\n");
}

static const char *std_pal[] = {
  "B", "B1", "G", "H", "I", "D", "D1", "K",
  "M", "N", "Nc", "60",
  NULL
};

static const char *std_ntsc[] = {
  "M", "M-JP", "443", "M-KR",
  NULL
};

static const char *std_secam[] = {
  "B", "D", "G", "H", "K", "K1", "L", "Lc",
  NULL
};

static const char *std_atsc[] = {
  "8-VSB", "16-VSB",
  NULL
};

int USBGrabber::validateCaptureDevice() {
  struct v4l2_capability          caps;
  clear(caps);

  // Get capturer capabilities
  if (v4l2_ioctl(mFd, VIDIOC_QUERYCAP, &caps)){
    error("Error accessing to capturer properties");
    return 0;
  }

  print("\nFound capturer %s\n", caps.card);
  print("Driver: %s\n\n", caps.driver);

  if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    error("Device doesn't support video capture\n");
    return 0;
  }

  if (!(caps.capabilities & V4L2_CAP_READWRITE)) {
    error("Device doesn't support read/write system calls\n");
    return 0;
  }

  if (!(caps.capabilities & V4L2_CAP_STREAMING)) {
    error("Device doesn't upport streaming\n");
    return 0;
  }

  return 1;
}

int USBGrabber::initCaptureFormat() {
  if (v4l2_ioctl(mFd, VIDIOC_G_FMT, &mFmt) < 0) {
    error("failed to determine video format\n");
    return 0;
  }

  return 1;
}

int USBGrabber::setCaptureFormat(size_t *aWidth, size_t *aHeight, uint32_t *aFormat) {
  // Set params
  if (*aWidth != 0)
    mFmt.fmt.pix.width       = *aWidth;
  else
    *aWidth = mFmt.fmt.pix.width;

  if (*aHeight != 0)
    mFmt.fmt.pix.height      = *aHeight;
  else
    *aHeight = mFmt.fmt.pix.height;

  if (*aFormat != 0)
    mFmt.fmt.pix.pixelformat = *aFormat;
  else
    *aFormat = mFmt.fmt.pix.pixelformat;

  if (*aWidth == 0 && *aHeight == 0 && *aFormat == 0)
    return 1;

  // Configure capture format
  if (v4l2_ioctl(mFd, VIDIOC_S_FMT, &mFmt) < 0) {
    error("Error setting capture format\n");
    return 0;
  }

  // Check changes
  if (mFmt.fmt.pix.pixelformat != *aFormat) {
    error("Error setting pixel format\n");
    return 0;
  }

  if ((mFmt.fmt.pix.width != *aWidth) || (mFmt.fmt.pix.height != *aHeight)) {
    error("Error setting capture size %zux%zu\n", *aWidth, *aHeight);
    return 0;
  }

  return 1;
}

int USBGrabber::getCaptureInfo() {
  v4l2_std_id std;

  // Get capturer input video standard
  if (v4l2_ioctl(mFd, VIDIOC_G_STD, &std) < 0) {
    error("Can't get capturer input video standard\n");
    return 0;
  }
 
  print("Video Standard: \n");
  if (std & 0xfff) {
    print("  PAL ");
    partstd2s(std_pal, std);
  }
  if (std & 0xf000) {
    print("  NTSC ");
    partstd2s(std_ntsc, std >> 12);
  }
  if (std & 0xff0000) {
    print("  SECAM ");
    partstd2s(std_secam, std >> 16);
  }
  if (std & 0xf000000) {
    print("  ATSC ");
    partstd2s(std_atsc, std >> 24);
  }

  // Get capturer formats
  print("Resolution: %dx%d\n", mFmt.fmt.pix.width,
                               mFmt.fmt.pix.height);

  print("Pixelformat: %c%c%c%c\n",
                                mFmt.fmt.pix.pixelformat & 0xFF,
                                (mFmt.fmt.pix.pixelformat >> 8) & 0xFF,
                                (mFmt.fmt.pix.pixelformat >> 16) & 0xFF,
                                (mFmt.fmt.pix.pixelformat >> 24) & 0xFF);

  print("Field: ");
  switch(mFmt.fmt.pix.field) {
    case V4L2_FIELD_ANY:           print("any\n");           break;
    case V4L2_FIELD_NONE:          print("none\n");          break;
    case V4L2_FIELD_TOP:           print("top\n");           break;
    case V4L2_FIELD_BOTTOM:        print("bottom\n");        break;
    case V4L2_FIELD_INTERLACED:    print("interlaced\n");    break;
    case V4L2_FIELD_SEQ_TB:        print("seq-tb\n");        break;
    case V4L2_FIELD_SEQ_BT:        print("seq-bt\n");        break;
    case V4L2_FIELD_ALTERNATE:     print("alternate\n");     break;
    case V4L2_FIELD_INTERLACED_TB: print("interlaced-tb\n"); break;
    case V4L2_FIELD_INTERLACED_BT: print("interlaced-bt\n"); break;
    default:                       print("unknown\n");       break;
  }

  print("Image size: %d bytes\n", mFmt.fmt.pix.sizeimage);

  print("Color space: ");
  switch (mFmt.fmt.pix.colorspace) {
    case V4L2_COLORSPACE_SMPTE170M:     print("Broadcast NTSC/PAL (SMPTE170M/ITU601)\n"); break;
    case V4L2_COLORSPACE_SMPTE240M:     print("1125-Line (US) HDTV (SMPTE240M)\n");     break;
    case V4L2_COLORSPACE_REC709:        print("HDTV and modern devices (ITU709)\n");       break;
    case V4L2_COLORSPACE_BT878:         print("Broken Bt878\n");    break;
    case V4L2_COLORSPACE_470_SYSTEM_M:  print("NTSC/PAL-M (ITU470/ITU601)\n");    break;
    case V4L2_COLORSPACE_470_SYSTEM_BG: print("PAL/SECAM BG (ITU470/ITU601)\n");  break;
    case V4L2_COLORSPACE_JPEG:          print("JPEG (JFIF/ITU601\n");     break;
    case V4L2_COLORSPACE_SRGB:          print("SRGB\n");      break;
    default:                            print("unknown\n");  break;
  }

  return 1;
}

int USBGrabber::initBuffers() {
  struct v4l2_requestbuffers req;
  struct v4l2_buffer         buf;

  // Configure capture buffers
  clear(req);
  req.count = 256;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (v4l2_ioctl(mFd, VIDIOC_REQBUFS, &req) < 0) {
    error("Can't initializate capture buffers\n");
    return 0;
  }

  if (req.count < 2) {
    error("Insufficient buffer memory\n");
    return 0;
  }

  mBuffers = req.count;
  mBuf = (buffer*)calloc(req.count, sizeof(struct buffer));
  for (size_t i = 0; i < req.count; i++) {
    clear(buf);

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = i;

    if (v4l2_ioctl(mFd, VIDIOC_QUERYBUF, &buf) < 0 ) {
      error("Error setting capture buffer\n");
      return 0;
    }

    mBuf[i].length = buf.length;
    mBuf[i].start = v4l2_mmap( NULL, buf.length,
                               PROT_READ | PROT_WRITE, MAP_SHARED,
                               mFd, buf.m.offset);

    if (mBuf[i].start == MAP_FAILED) {
      error("Error mapping capture device to buffer\n");
      return 0;
    }
  }

  return 1;
}

int USBGrabber::startStreaming() {
  struct v4l2_buffer buf;

  for (size_t i = 0; i < mBuffers; i++) {
    clear(buf);

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = i;

   if (v4l2_ioctl(mFd, VIDIOC_QBUF, &buf) < 0 ) {
      error("Error exchanging capture buffer with the driver\n");
      return 0;
    }
  }

  if (v4l2_ioctl(mFd, VIDIOC_STREAMON, &mType) < 0 ) {
    error("Can't start streaming I/O\n");
    return 0;
  }

  return 1;
}
