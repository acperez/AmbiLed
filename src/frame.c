#include "frame.h"
#include "common.h"

typedef struct {
  unsigned char R;
  unsigned char G;
  unsigned char B;
} RGBVal;

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

RGBVal getAverage(unsigned char *data, Rect mask, int width) {
  int R = 0;
  int G = 0;
  int B = 0;
  int counter = 0;

  for (int x = mask.x; x < mask.x + mask.width; x++)  {
    for (int y = mask.y; y < mask.y + mask.height; y++) {
      int pixel = x * 3 + y * width * 3;
      R += data[pixel];
      G += data[pixel + 1];
      B += data[pixel + 2];
      counter++;
    }
  }

  return (RGBVal){R / counter, G / counter, B / counter};
}

void paintLed(unsigned char *data, Rect mask, RGBVal color, int width) {
  for (int x = mask.x; x < mask.x + mask.width; x++)  {
    for (int y = mask.y; y < mask.y + mask.height; y++) {
      int pixel = x * 3 + y * width * 3;
      data[pixel] = color.R;
      data[pixel + 1] = color.G;
      data[pixel + 2] = color.B;
    }
  }
}

unsigned char* processFrame(Frame *frame, Leds *leds) {
  unsigned char *output = (unsigned char *) malloc(frame->bytes);
  memset(output, 0, frame->bytes);

  for (int i = 0; i < leds->leds; i++) {
    RGBVal color = getAverage(frame->data, leds->masks[i], frame->width);
    paintLed(output, leds->masks[i], color, frame->width);
  }

  return output;
}
