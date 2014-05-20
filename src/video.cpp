/*#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus 
}
#endif
*/
#include "video.h"

Video::Video(): mBuffer(0),
                mFrameRGB(0),
                mFrame(0),
                mCodecCtx(0),
                mFormatCtx(0),
                mSws_ctx(0),
                mVideoStream(-1) {
}

Video::~Video() {
  // Free the RGB image
  av_free(mBuffer);
  av_free(mFrameRGB);

  // Free the YUV frame
  av_free(mFrame);

  // Close the codec
  avcodec_close(mCodecCtx);

  // Close the video file
  avformat_close_input(&mFormatCtx);

  free(mSws_ctx);
}

int Video::open(const char* aFilePath, size_t *aWidth, size_t *aHeight, uint32_t *aFormat) {
  if (mFormatCtx != 0) {
    error("Video stream already initialized\n");
    return 0;
  }

  AVCodec      *pCodec = 0;
  int          numBytes;

  AVDictionary *optionsDict = 0;

  // Register all formats and codecs
  av_register_all();
  
  // Open video file
  if(avformat_open_input(&mFormatCtx, aFilePath, NULL, NULL)!=0)
    return 0; // Couldn't open file
  
  // Retrieve stream information
  if(avformat_find_stream_info(mFormatCtx, NULL)<0)
    return 0; // Couldn't find stream information
  
  // Dump information about file onto standard error
  av_dump_format(mFormatCtx, 0, aFilePath, 0);
  
  // Find the first video stream
  for(size_t i = 0; i < mFormatCtx->nb_streams; i++)
    if(mFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      mVideoStream=i;
      break;
    }
  if(mVideoStream==-1)
    return 0; // Didn't find a video stream
  
  // Get a pointer to the codec context for the video stream
  mCodecCtx = mFormatCtx->streams[mVideoStream]->codec;
  
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(mCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return 0; // Codec not found
  }
  // Open codec
  if(avcodec_open2(mCodecCtx, pCodec, &optionsDict)<0)
    return 0; // Could not open codec
  
  // Allocate video frame
  mFrame=avcodec_alloc_frame();
  
  // Allocate an AVFrame structure
  mFrameRGB=avcodec_alloc_frame();
  if(mFrameRGB==NULL)
    return 0;
  
  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(PIX_FMT_RGB24, *aWidth,
			      *aHeight);
  mBuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

  mSws_ctx =
    sws_getContext
    (
        mCodecCtx->width,
        mCodecCtx->height,
        mCodecCtx->pix_fmt,
        *aWidth,
        *aHeight,
        PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );
  
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)mFrameRGB, mBuffer, PIX_FMT_RGB24,
		 *aWidth, *aHeight);
  
  mWidth = *aWidth;
  mHeight = *aHeight;

  return 1;
}

int Video::getFrame(uint8_t *aData, size_t aBytes) {
  AVPacket        packet;
  int             frameFinished;

  // Read frames and save first five frames to disk
  while (av_read_frame(mFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if (packet.stream_index == mVideoStream) {
      // Decode video frame
      avcodec_decode_video2(mCodecCtx, mFrame, &frameFinished,
                           &packet);

      // Did we get a video frame?
      if(frameFinished) {
        // Convert the image from its native format to RGB
        sws_scale
        (
            mSws_ctx,
            (uint8_t const * const *)mFrame->data,
            mFrame->linesize,
            0,
            mCodecCtx->height,
            mFrameRGB->data,
            mFrameRGB->linesize
        );

        if (aBytes != mHeight * mWidth * 3) {
          error("Capturing format mismatch");
          return 0;
        }

        memcpy(aData, mFrameRGB->data[0], mHeight * mWidth * 3);                      
        break;
      }
    }

    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }

  return 1;
}
