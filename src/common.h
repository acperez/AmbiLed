#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdint.h>

#define clear(x) memset(&(x), 0, sizeof(x))

#define print(FMT, ARGS...) fprintf(stdout, FMT, ## ARGS)
#define error(FMT, ARGS...) fprintf(stderr, FMT, ## ARGS)

enum VideoType { CAPTURER, PLAYER, UNKNOWN };
enum LedControllerType { ARDUINO, RASPBERRY, NONE };
enum LedType { WS2801, WS2812B, TM1804, EMPTY };

typedef struct {
  int x;
  int y;
  int width;
  int height;
} Rect;


typedef struct RGBVal {
    uint8_t R;
    uint8_t G;
    uint8_t B;

    RGBVal(uint8_t aR, uint8_t aG, uint8_t aB): R(aR),
                                                G(aG),
                                                B(aB) {
    }
} RGBVal;

#endif
