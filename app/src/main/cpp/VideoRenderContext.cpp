#include "VideoRenderContext.h"

VideoRenderContext::jni_fields_t VideoRenderContext::jni_fields = { 0L };

VideoRenderContext::VideoRenderContext()
{
    m_pVideoRender = VideoRender::create(tYUV420);
}

VideoRenderContext::~VideoRenderContext()
{

}

void VideoRenderContext::render()
{
	m_pVideoRender->render();
}

void VideoRenderContext::draw(uint8_t *buffer, size_t length, size_t width, size_t height)
{
    m_pVideoRender->draw(buffer, length, width, height);
}

void VideoRenderContext::createContext(JNIEnv *env, jobject obj)
{
    VideoRenderContext* context = new VideoRenderContext();

    storeContext(env, obj, context);
}

void VideoRenderContext::storeContext(JNIEnv *env, jobject obj, VideoRenderContext *context)
{
    // Get a reference to this object's class
    jclass cls = env->GetObjectClass(obj);

    if (NULL == cls)
    {
        LOGE("Could not find com/media/camera2glpreview/render/VideoRender.");
        return;
    }

    // Get the Field ID of the "mNativeContext" variable
    jni_fields.context = env->GetFieldID(cls, "mNativeContext", "J");
    if (NULL == jni_fields.context)
    {
        LOGE("Could not find mNativeContext.");
        return;
    }

    env->SetLongField(obj, jni_fields.context, (jlong)context);
}

void VideoRenderContext::deleteContext(JNIEnv *env, jobject obj)
{
    if (NULL == jni_fields.context)
    {
        LOGE("Could not find mNativeContext.");
        return;
    }

    VideoRenderContext* context = reinterpret_cast<VideoRenderContext*>(env->GetLongField(obj, jni_fields.context));

    if (context) delete context;

    env->SetLongField(obj, jni_fields.context, 0L);
}

VideoRenderContext* VideoRenderContext::getContext(JNIEnv *env, jobject obj)
{
    if (NULL == jni_fields.context)
    {
        LOGE("Could not find mNativeContext.");
        return NULL;
    }

    VideoRenderContext* context = reinterpret_cast<VideoRenderContext*>(env->GetLongField(obj, jni_fields.context));

    return context;
}
