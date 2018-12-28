//
// Created by yijunwu on 2018/12/18.
//

#ifndef LIVERECORD_LIVE_X264_H
#define LIVERECORD_LIVE_X264_H

#include <pthread.h>
#include <libx264/x264.h>
#include "queue.h"
#include "live_def.h"
#include "live_rtmp.h"

typedef struct _live_x264 LiveX264;

struct _live_x264 {
    LiveRtmp *liveRtmp;
    pthread_t t;
    Queue *queue;

    x264_picture_t *pic_in;
    x264_picture_t *pic_out;
    x264_t *encoder;

    //原图的宽高
    int width;
    int height;
    int bitrate;
    int orientation;

    int pts;

    uint8_t *sps;
    int sps_len;
    uint8_t *pps;
    int pps_len;

    void (*video_call)(x264_nal_t *nals, int num_nals);
};

int x264_init(LiveX264 *liveX264, int mWidth, int mHeight, int bitrate, int orientation);

int x264_encode(LiveX264 *liveX264, char *inBytes, int pts, char *outBytes, int *outFrameSize);

int x264_release(LiveX264 *liveX264);

#endif //LIVERECORD_LIVE_X264_H
