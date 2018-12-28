//
// Created by yijunwu on 2018/12/7.
//

#ifndef LIVERECORD_LIVE_FDK_AAC_H
#define LIVERECORD_LIVE_FDK_AAC_H

#include <pthread.h>
#include <fdk-aac/aacenc_lib.h>
#include "log.h"
#include "queue.h"
#include "live_def.h"
#include "live_rtmp.h"


struct _live_aac {
    LiveRtmp *liveRtmp;
    pthread_t t;
    Queue *queue;

    HANDLE_AACENCODER handle;
    int channels;
    int sample_rate;
    int bitRate; //64000

} typedef LiveAAC;

int aac_init(LiveAAC *liveAAC, int channels, int sampleRate, int bitRate);

int aac_encode_audio(LiveAAC *liveAAC, unsigned char *convert_buf, int read, unsigned char *outbuf, int outLength);

int aac_release(LiveAAC *liveAAC);

#endif //LIVERECORD_LIVE_FDK_AAC_H
