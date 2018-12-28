//
// Created by yijunwu on 2018/12/19.
//

#ifndef LIVERECORD_YUV_UTIL_H
#define LIVERECORD_YUV_UTIL_H

#include <memory.h>

void NV21ToI420(char *dstData, char *srcdata, int len);

void YUV420spRotateNegative90(char *dst, const char *src, int width, int height);

void rotate90(int start, int width, int height, char *dst, char *src);

void YUV420Rotate90(char *dst, char *src, int width, int height);

#endif //LIVERECORD_YUV_UTIL_H
