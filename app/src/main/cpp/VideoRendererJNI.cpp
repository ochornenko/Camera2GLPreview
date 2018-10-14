#include "VideoRendererJNI.h"
#include "VideoRendererContext.h"

JCMCRV(void, create)(JNIEnv * env, jobject obj)
{
    VideoRendererContext::createContext(env, obj);
}

JCMCRV(void, destroy)(JNIEnv * env, jobject obj)
{
    VideoRendererContext::deleteContext(env, obj);
}

JCMCRV(void, init)(JNIEnv * env, jobject obj,  jint width, jint height)
{
    VideoRendererContext* context = VideoRendererContext::getContext(env, obj);

    if (context) context->init((size_t)width, (size_t)height);
}

JCMCRV(void, render)(JNIEnv * env, jobject obj)
{
	VideoRendererContext* context = VideoRendererContext::getContext(env, obj);

	if (context) context->render();
}

JCMCRV(void, draw)(JNIEnv * env, jobject obj, jbyteArray data, jint width, jint height, jint rotation)
{
	jbyte* bufferPtr = env->GetByteArrayElements(data, 0);

	jsize arrayLength = env->GetArrayLength(data);

	VideoRendererContext* context = VideoRendererContext::getContext(env, obj);

	if (context) context->draw((uint8_t *)bufferPtr, (size_t)arrayLength, (size_t)width, (size_t)height, rotation);

	env->ReleaseByteArrayElements(data, bufferPtr, 0);
}

JCMCRV(void, applyFilter)(JNIEnv * env, jobject obj, jint filter)
{
	VideoRendererContext* context = VideoRendererContext::getContext(env, obj);

	if (context) context->applyFilter(filter);
}

JCMCRV(jint, getMaxFilter)(JNIEnv * env, jobject obj)
{
	VideoRendererContext* context = VideoRendererContext::getContext(env, obj);

	if (context) return context->getMaxFilter();

	return 0;
}
