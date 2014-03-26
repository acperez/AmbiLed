#ifndef LEDS_H
#define LEDS_H

#include <stdlib.h>

typedef struct {
  int x;
  int y;
  int width;
  int height;
} Rect;

typedef struct {
  int leds;
  Rect *masks;
} Leds;

Leds* initLeds(int leds_x, int leds_y, int width, int height);
void freeLeds(Leds *leds);

#endif
