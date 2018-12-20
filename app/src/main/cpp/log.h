//
// Created by david on 2017/9/27.
//

#ifndef LOG_H
#define LOG_H
#include <android/log.h>

#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"yijun",FORMAT,##__VA_ARGS__);

#define LOGER(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"yijun_rtmp",FORMAT,##__VA_ARGS__);

#endif //LOG_H
