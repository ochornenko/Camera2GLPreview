#include "VideoRenderJNI.h"
#include "VideoRenderContext.h"

JCMCRV(void, create)(JNIEnv * env, jobject obj)
{
    VideoRenderContext::createContext(env, obj);
}

JCMCRV(void, destroy)(JNIEnv * env, jobject obj)
{
    VideoRenderContext::deleteContext(env, obj);
}

JCMCRV(void, init)(JNIEnv * env, jobject obj,  jint width, jint height)
{

}

JCMCRV(void, render)(JNIEnv * env, jobject obj)
{
	VideoRenderContext* context = VideoRenderContext::getContext(env, obj);

	if (context) context->render();
}

JCMCRV(void, draw)(JNIEnv * env, jobject obj, jbyteArray data, jint width, jint height)
{
	jbyte* bufferPtr = env->GetByteArrayElements(data, 0);

	jsize arrayLength = env->GetArrayLength(data);

	VideoRenderContext* context = VideoRenderContext::getContext(env, obj);

	if (context) context->draw((uint8_t *) bufferPtr, (size_t)arrayLength, (size_t)width, (size_t)height);

	env->ReleaseByteArrayElements(data, bufferPtr, 0);
}
