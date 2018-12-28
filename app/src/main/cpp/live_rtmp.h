//
// Created by yijunwu on 2018/12/3.
//

#ifndef LIVERECORD_LIVE_RTMP_H
#define LIVERECORD_LIVE_RTMP_H

#include <librtmp/rtmp.h>
#include "queue.h"
#include "live_def.h"


#define TYPE_NV21 1
#define TYPE_PCM 2

struct _live_rtmp {
    RTMP *rtmp;
    uint32_t start_time;

    bool is_start;
    pthread_t t;
    Queue *queue;


} typedef LiveRtmp;

struct _live_package {
    int type;
    char *data;
    int length;
} typedef LivePackage;

void rtmp_init(LiveRtmp *liveRtmp, unsigned char *url);

void pushSPSPPS(LiveRtmp *liveRtmp, char *sps, int spsLen, char *pps, int ppsLen);

void pushVideoData(LiveRtmp *liveRtmp, char *data, int dataLen, int keyFrame);

void pushAudioData(LiveRtmp *liveRtmp, char *data, int dataLen);

void rtmp_release(LiveRtmp *liveRtmp);

#endif //LIVERECORD_LIVE_RTMP_H
