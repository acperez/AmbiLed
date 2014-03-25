#ifndef FRAME_H
#define FRAME_H

#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned char *data;
  int bytes;
  int format;
  int width;
  int height;
} Frame;

Frame* new_Frame(int format, int bytes, int width, int height); 
void free_Frame(Frame *frame);

unsigned char* getData(Frame *frame);
void setData(Frame *frame, void *start, int bytes);
int processFrame();

#endif
