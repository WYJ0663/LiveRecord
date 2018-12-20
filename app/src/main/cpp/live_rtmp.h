//
// Created by yijunwu on 2018/12/3.
//

#ifndef LIVERECORD_LIVE_RTMP_H
#define LIVERECORD_LIVE_RTMP_H

#include <librtmp/rtmp.h>

struct _live_rtmp {
    RTMP *rtmp;
    uint32_t start_time;
} typedef LiveRtmp;

void rtmp_init(LiveRtmp *liveRtmp, unsigned char *url);

void pushSPSPPS(LiveRtmp *liveRtmp, char *sps, int spsLen, char *pps, int ppsLen);

void pushVideoData(LiveRtmp *liveRtmp, char *data, int dataLen, int keyFrame);

void pushAudioData(LiveRtmp *liveRtmp, char *data, int dataLen);

void rtmp_release(LiveRtmp *liveRtmp);

#endif //LIVERECORD_LIVE_RTMP_H
