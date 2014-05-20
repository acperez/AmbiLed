#ifndef VIDEO_H
#define VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus 
}
#endif

#include "common.h"
#include "capturer.h"

class Video: public Capturer {
  public:
    Video();
    ~Video();

    int open(const char* aFilePath, size_t *aWidth, size_t *aHeight, uint32_t *aFormat);
    int getFrame(uint8_t *aData, size_t aBytes);

  private:
    uint8_t *mBuffer;
    AVFrame *mFrameRGB;
    AVFrame *mFrame;
    AVCodecContext *mCodecCtx;
    AVFormatContext *mFormatCtx;
    struct SwsContext *mSws_ctx;
    int mVideoStream;
    size_t mWidth;
    size_t mHeight;
};

#endif
