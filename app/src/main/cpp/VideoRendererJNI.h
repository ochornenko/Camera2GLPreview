#ifndef _H_VIDEO_RENDER_JNI_
#define _H_VIDEO_RENDER_JNI_

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JCMCRV(rettype, name)                                             \
  rettype JNIEXPORT JNICALL Java_com_media_camera2glpreview_render_VideoRenderer_##name

JCMCRV(void, create)(JNIEnv * env, jobject obj);
JCMCRV(void, destroy)(JNIEnv * env, jobject obj);
JCMCRV(void, init)(JNIEnv * env, jobject obj,  jint width, jint height);
JCMCRV(void, render)(JNIEnv * env, jobject obj);
JCMCRV(void, draw)(JNIEnv * env, jobject obj, jbyteArray data, jint width, jint height, jint rotation);
JCMCRV(void, applyFilter)(JNIEnv * env, jobject obj, jint filter);
JCMCRV(jint, getMaxFilter)(JNIEnv * env, jobject obj);

#ifdef __cplusplus
}
#endif

#endif // _H_VIDEO_RENDER_JNI_
