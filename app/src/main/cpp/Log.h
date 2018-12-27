#ifndef _LOG_H_
#define _LOG_H_

#include <android/log.h>

#define DEBUG 1

#define  LOG_TAG "media-lib"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#if DEBUG
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(...)
#endif

#endif // _LOG_H_
