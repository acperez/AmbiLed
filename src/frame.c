#include "frame.h"
#include "common.h"

Frame* new_Frame(int format, int bytes, int width, int height) {
  Frame* frame = (Frame*)malloc(sizeof(Frame));
  frame->data = (unsigned char *) malloc(bytes);
  memset(frame->data, 0, bytes);
  frame->bytes = bytes;
  frame->format = format;
  frame->width = width;
  frame->height = height;

  return frame;
}

void free_Frame(Frame *frame) {
  free(frame->data);
  free(frame);
}

int processFrame() {
  return 0;
}
