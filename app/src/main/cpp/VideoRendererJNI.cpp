#include "VideoRendererJNI.h"
#include "VideoRendererContext.h"

#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>

JCMCPRV(void, create)(JNIEnv *env, jobject obj, jint type) {
    VideoRendererContext::createContext(env, obj, type);
}

JCMCPRV(void, destroy)(JNIEnv *env, jobject obj) {
    VideoRendererContext::deleteContext(env, obj);
}

JCMCPRV(void, init)(JNIEnv *env, jobject obj, jobject surface, jobject assetManager, jint width, jint height) {
    VideoRendererContext *context = VideoRendererContext::getContext(env, obj);

    ANativeWindow *window = surface ? ANativeWindow_fromSurface(env, surface) : nullptr;

    auto *aAssetManager = assetManager ? AAssetManager_fromJava(env, assetManager) : nullptr;

    if (context) context->init(window, aAssetManager, (size_t) width, (size_t) height);
}

JCMCPRV(void, render)(JNIEnv *env, jobject obj) {
    VideoRendererContext *context = VideoRendererContext::getContext(env, obj);

    if (context) context->render();
}

JCMCPRV(void, draw)(JNIEnv *env, jobject obj, jbyteArray data, jint width, jint height,
                    jint rotation, jboolean mirror) {
    jbyte *bufferPtr = env->GetByteArrayElements(data, nullptr);

    jsize arrayLength = env->GetArrayLength(data);

    VideoRendererContext *context = VideoRendererContext::getContext(env, obj);

    if (context) context->draw((uint8_t *) bufferPtr, (size_t) arrayLength, (size_t) width, (size_t) height, rotation, mirror);

    env->ReleaseByteArrayElements(data, bufferPtr, 0);
}

JCMCPRV(void, setParameters)(JNIEnv *env, jobject obj, jint params) {
    VideoRendererContext *context = VideoRendererContext::getContext(env, obj);

    if (context) context->setParameters((uint32_t) params);
}

JCMCPRV(jint, getParameters)(JNIEnv *env, jobject obj) {
    VideoRendererContext *context = VideoRendererContext::getContext(env, obj);

    if (context) return context->getParameters();

    return 0;
}
