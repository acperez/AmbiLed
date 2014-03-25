#include "capture.h"
#include "common.h"

struct buffer
{
  void   *start;
  size_t length;
};

typedef struct {
  int fd;
  struct v4l2_format fmt;
  struct buffer *buf;
  int buffers;
  enum v4l2_buf_type type;
} capturer;


const capturer initCapturer() {
  capturer device;

  device.fd       = 0;
  device.buf      = 0;
  device.buffers  = 0;
  device.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  clear(device.fmt);
  device.fmt.type = device.type;

  return device;
}

static capturer device;

int validateCaptureDevice();
int initCaptureFormat();
int setCaptureFormat(int *width, int *height, int *format);
int getCaptureInfo();
int initBuffers();
int startStreaming();


int initCaptureDevice(const char* devPath, int *width, int *height, int *format) {
  if (device.fd != 0) {
    error("Capture device already initialized\n");
    return 0;
  }

  // init device struc
  device = initCapturer();

  // Open capture device
  device.fd = v4l2_open(devPath, O_RDWR | O_NONBLOCK, 0);
  if (device.fd < 0)
  {
    error("Cannot open capturer device\n");
    return 0;
  }

  if (!validateCaptureDevice()) return 0;
  if (!initCaptureFormat()) return 0;
  if (!setCaptureFormat(width, height, format)) return 0;
  if (!getCaptureInfo()) return 0;
  if (!initBuffers()) return 0;
  if (!startStreaming()) return 0;

  return 1;
}

void closeCaptureDevice()
{
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl(device.fd, VIDIOC_STREAMOFF, &type) < 0 ) {
    error("Can't stop streaming I/O\n");
  }

  for (int i = 0; i < device.buffers; i++)
  {
    v4l2_munmap(device.buf[i].start, device.buf[i].length);
  }

  free(device.buf);
  v4l2_close(device.fd);
}

int captureFromDevice(Frame *frame) {
  struct v4l2_buffer buf;
  fd_set         fds;
  struct timeval tv;
  int            r;

  do {
    FD_ZERO(&fds);
    FD_SET(device.fd, &fds);

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    r = select(device.fd + 1, &fds, NULL, NULL, &tv);
  } while ((r == -1 && (errno = EINTR)));

  if (r == -1) {
    error("select error %d, %s\n", errno, strerror(errno));
    return -1;
  }

  clear(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  // Enqueue an empty (capturing) buffer in the driver's incoming queue.
  if (v4l2_ioctl(device.fd, VIDIOC_DQBUF, &buf) < 0 ) {
    error("Can't queue buffer\n");
    return 0;
  }

  if (buf.index >= device.buffers) {
    error("Invalid buffer index received\n");
    return 0;
  }

  if (buf.bytesused != frame->bytes) {
    error("Capturing format mismatch\n");
    return 0;
  }
  memcpy(frame->data, device.buf[buf.index].start, buf.bytesused);

  // Dequeue a filled (capturing) buffer from the driver's outgoing queue.
  if (v4l2_ioctl(device.fd, VIDIOC_QBUF, &buf) < 0 ) {
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

int validateCaptureDevice() {
  struct v4l2_capability          caps;
  clear(caps);

  // Get capturer capabilities
  if (v4l2_ioctl(device.fd, VIDIOC_QUERYCAP, &caps)){
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

int initCaptureFormat() {
  if (v4l2_ioctl(device.fd, VIDIOC_G_FMT, &device.fmt) < 0) {
    error("failed to determine video format\n");
    return 0;
  }

  return 1;
}

int setCaptureFormat(int *width, int *height, int *format) {
  // Set params
  if (*width != -1)
    device.fmt.fmt.pix.width       = *width;
  else
    *width = device.fmt.fmt.pix.width;

  if (*height != -1)
    device.fmt.fmt.pix.height      = *height;
  else
    *height = device.fmt.fmt.pix.height;

  if (*format != -1)
    device.fmt.fmt.pix.pixelformat = *format;
  else
    *format = device.fmt.fmt.pix.pixelformat;

  if (*width == -1 && *height == -1 && *format == -1)
    return 1;

  // Configure capture format
  if (v4l2_ioctl(device.fd, VIDIOC_S_FMT, &device.fmt) < 0) {
    error("Error setting capture format\n");
    return 0;
  }

  // Check changes
  if (device.fmt.fmt.pix.pixelformat != *format) {
    error("Error setting pixel format\n");
    return 0;
  }

  if ((device.fmt.fmt.pix.width != *width) || (device.fmt.fmt.pix.height != *height)) {
    error("Error setting capture size %dx%d\n", *width, *height);
    return 0;
  }

  return 1;
}

int getCaptureInfo() {
  v4l2_std_id std;

  // Get capturer input video standard
  if (v4l2_ioctl(device.fd, VIDIOC_G_STD, &std) < 0) {
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
  print("Resolution: %dx%d\n", device.fmt.fmt.pix.width,
                               device.fmt.fmt.pix.height);

  print("Pixelformat: %c%c%c%c\n",
                                device.fmt.fmt.pix.pixelformat & 0xFF,
                                (device.fmt.fmt.pix.pixelformat >> 8) & 0xFF,
                                (device.fmt.fmt.pix.pixelformat >> 16) & 0xFF,
                                (device.fmt.fmt.pix.pixelformat >> 24) & 0xFF);

  print("Field: ");
  switch(device.fmt.fmt.pix.field) {
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

  print("Image size: %d bytes\n", device.fmt.fmt.pix.sizeimage);

  print("Color space: ");
  switch (device.fmt.fmt.pix.colorspace) {
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

int initBuffers() {
  struct v4l2_requestbuffers req;
  struct v4l2_buffer         buf;

  // Configure capture buffers
  clear(req);
  req.count = 1;
  req.count = 256;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (v4l2_ioctl(device.fd, VIDIOC_REQBUFS, &req) < 0) {
    error("Can't initializate capture buffers\n");
    return 0;
  }

  if (req.count < 2) {
    error("Insufficient buffer memory\n");
    return 0;
  }

  device.buffers = req.count;
  device.buf = calloc(req.count, sizeof(struct buffer));
  for (int i = 0; i < req.count; i++)
  {
    clear(buf);

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = i;

    if (v4l2_ioctl(device.fd, VIDIOC_QUERYBUF, &buf) < 0 ) {
      error("Error setting capture buffer\n");
      return 0;
    }

    device.buf[i].length = buf.length;
    device.buf[i].start = v4l2_mmap( NULL, buf.length,
                                        PROT_READ | PROT_WRITE, MAP_SHARED,
                                        device.fd, buf.m.offset);

    if (device.buf[i].start == MAP_FAILED) {
      error("Error mapping capture device to buffer\n");
      return 0;
    }
  }

  return 1;
}

int startStreaming() {
  struct v4l2_buffer buf;

  for (int i = 0; i < device.buffers; i++) {
    clear(buf);

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = i;

   if (v4l2_ioctl(device.fd, VIDIOC_QBUF, &buf) < 0 ) {
      error("Error exchanging capture buffer with the driver\n");
      return 0;
    }
  }

  if (v4l2_ioctl(device.fd, VIDIOC_STREAMON, &device.type) < 0 ) {
    error("Can't start streaming I/O\n");
    return 0;
  }

  return 1;
}
