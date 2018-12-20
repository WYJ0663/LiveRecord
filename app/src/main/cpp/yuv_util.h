//
// Created by yijunwu on 2018/12/19.
//

#ifndef LIVERECORD_YUV_UTIL_H
#define LIVERECORD_YUV_UTIL_H

#include <memory.h>

void NV21ToI420(char *dstData, char *srcdata, int len) {

    int size = len * 4 / 6;//NV12:YYYYVU
    // Y
    memcpy(dstData, srcdata, len * 4 / 6);
    for (int i = 0; i < size / 4; i++) {
        dstData[size + i] = srcdata[size + i * 2 + 1]; //U
        dstData[size + size / 4 + i] = srcdata[size + i * 2]; //V
    }
}

void YUV420spRotateNegative90(char *dst, const char *src, int width, int height) {
    int nWidth = 0, nHeight = 0;
    int wh = 0;
    int uvHeight = 0;
    if (width != nWidth || height != nHeight) {
        nWidth = width;
        nHeight = height;
        wh = width * height;
        uvHeight = height >> 1;//uvHeight = height / 2
    }

    //旋转Y
    int k = 0;
    for (int i = 0; i < width; i++) {
        int nPos = width - 1;
        for (int j = 0; j < height; j++) {
            dst[k] = src[nPos - i];
            k++;
            nPos += width;
        }
    }

    for (int i = 0; i < width; i += 2) {
        int nPos = wh + width - 1;
        for (int j = 0; j < uvHeight; j++) {
            dst[k] = src[nPos - i - 1];
            dst[k + 1] = src[nPos - i];
            k += 2;
            nPos += width;
        }
    }
    return;
}

void rotate90(int start, int width, int height, char *dst, char *src) {
    int wh = width * height;
    int k = start;
    for (int i = 0; i < width; i++) {
        int nPos = wh - width + i + start;
        for (int j = 0; j < height; j++) {
            dst[k] = src[nPos];
            k++;
            nPos -= width;
        }
    }
}

void YUV420Rotate90(char *dst, char *src, int width, int height) {

    int wh = width * height;
    int uvWidth = width >> 1;
    int uvHeight = height >> 1;

    rotate90(0, width, height, dst, src);
    rotate90(wh, uvWidth, uvHeight, dst, src);
    rotate90(wh * 5 / 4, uvWidth, uvHeight, dst, src);

    return;
}

#endif //LIVERECORD_YUV_UTIL_H
