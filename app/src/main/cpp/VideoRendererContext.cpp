#include "VideoRendererContext.h"

VideoRendererContext::jni_fields_t VideoRendererContext::jni_fields = { 0L };

VideoRendererContext::VideoRendererContext()
{
    m_pVideoRenderer = VideoRenderer::create(tYUV420_FILTER);
}

VideoRendererContext::~VideoRendererContext()
{

}

void VideoRendererContext::init(size_t width, size_t height)
{
    m_pVideoRenderer->init(width, height);
}

void VideoRendererContext::render()
{
	m_pVideoRenderer->render();
}

void VideoRendererContext::draw(uint8_t *buffer, size_t length, size_t width, size_t height, int rotation)
{
    m_pVideoRenderer->draw(buffer, length, width, height, rotation);
}

void VideoRendererContext::applyFilter(int filter)
{
    m_pVideoRenderer->applyFilter(filter);
}

int VideoRendererContext::getMaxFilter()
{
    return m_pVideoRenderer->getMaxFilter();
}

void VideoRendererContext::createContext(JNIEnv *env, jobject obj)
{
    VideoRendererContext* context = new VideoRendererContext();

    storeContext(env, obj, context);
}

void VideoRendererContext::storeContext(JNIEnv *env, jobject obj, VideoRendererContext *context)
{
    // Get a reference to this object's class
    jclass cls = env->GetObjectClass(obj);

    if (NULL == cls)
    {
        LOGE("Could not find com/media/camera2glpreview/render/VideoRenderer.");
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

void VideoRendererContext::deleteContext(JNIEnv *env, jobject obj)
{
    if (NULL == jni_fields.context)
    {
        LOGE("Could not find mNativeContext.");
        return;
    }

    VideoRendererContext* context = reinterpret_cast<VideoRendererContext*>(env->GetLongField(obj, jni_fields.context));

    if (context) delete context;

    env->SetLongField(obj, jni_fields.context, 0L);
}

VideoRendererContext* VideoRendererContext::getContext(JNIEnv *env, jobject obj)
{
    if (NULL == jni_fields.context)
    {
        LOGE("Could not find mNativeContext.");
        return NULL;
    }

    VideoRendererContext* context = reinterpret_cast<VideoRendererContext*>(env->GetLongField(obj, jni_fields.context));

    return context;
}
