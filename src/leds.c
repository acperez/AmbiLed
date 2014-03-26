#include "leds.h"
#include "common.h"

#define MASK_WIDTH 20
#define MASK_HEIGHT 10

Rect getMask(int x, int y, int maxWidth, int maxHeight)
{
  int rectangleX = MASK_WIDTH;
  int rectangleY = MASK_HEIGHT;

  int x0 = x - (rectangleX / 2);
  int y0 = y - (rectangleY / 2);

  int offsetX = 0;
  int offsetY = 0;

  if (x0 < 0)
    offsetX = x0 - (2 * x0);

  if (x0 + rectangleX > maxWidth - 1)
    offsetX = maxWidth - (x0 + rectangleX);

  if (y0 < 0)
    offsetY = y0 - (2 * y0);

  if (y0 + rectangleY > maxHeight - 1)
    offsetY = maxHeight - (y0 + rectangleY);

  return (Rect){x0 + offsetX, y0 + offsetY, rectangleX, rectangleY};
}

Leds* initLeds(int leds_x, int leds_y, int width, int height) {
  Leds* leds = (Leds*)malloc(sizeof(Leds));
  leds->leds = leds_x * 2 + leds_y * 2;

  Rect *masks = (Rect *) malloc((leds->leds) * sizeof(Rect));
  int index = 0;

  // Horizontal leds / stream pixels mapping
  int distance = width / (leds_x - 1);
  int offset = (width - (distance * (leds_x - 1))) / 2;
  for (int i = 0; i < leds_x; i++) {
    int x = (i * distance) + offset;
    masks[index] = getMask(x, 0, width, height);
    masks[index + leds_y + leds_x] = getMask(x, height - 1, width, height);
    index++;
  }

  // Vertical leds / stream pixels mapping
  distance = height / (leds_y - 1);
  offset = (height - (distance * (leds_y - 1))) / 2;
  for (int i = 0; i < leds_y; i++) {
    int y = (i * distance) + offset;
    masks[index] = getMask(0, y, width, height);
    masks[index + leds_x + leds_y] = getMask(width - 1, y, width, height);
    index++;
  }

  leds->masks = masks;

  return leds;
}

void freeLeds(Leds *leds) {
  free(leds->masks);
  free(leds);
}
