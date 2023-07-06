

#ifndef COMMON_H
#define COMMON_H

#include <android/log.h>
#include <cstdio>
#include <cstdlib>
#include <GLES3/gl3.h>

#include <string>
#include <vector>
#include <stdint.h>
#include "matrix.h"
typedef uint32_t  uint32;
typedef uint16_t  uint16;
typedef uint8_t   uint8;
typedef int32_t   int32;
typedef int16_t   int16;
typedef int8_t    int8;
using std::string;
using std::vector;

#define LOG_TAG "libNative"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define ASSERT(x, s)                                                    \
    if (!(x))                                                           \
    {                                                                   \
        LOGE("Assertion failed at %s:%i\n%s\n", __FILE__, __LINE__, s); \
        exit(1);                                                        \
    }

#endif /* COMMON_H */
