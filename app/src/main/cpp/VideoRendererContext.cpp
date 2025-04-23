#include "VideoRendererContext.h"
#include "Log.h"

VideoRendererContext::jni_fields_t VideoRendererContext::jni_fields = {nullptr};

VideoRendererContext::VideoRendererContext(int type) {
    m_pVideoRenderer = VideoRenderer::create(type);
}

VideoRendererContext::~VideoRendererContext() = default;

void VideoRendererContext::init(ANativeWindow *window, AAssetManager *assetManager, size_t width, size_t height) {
    m_pVideoRenderer->init(window, assetManager, width, height);
}

void VideoRendererContext::render() {
    m_pVideoRenderer->render();
}

void VideoRendererContext::draw(uint8_t *buffer, size_t length, size_t width, size_t height,
                                float rotation, bool mirror) {
    m_pVideoRenderer->draw(buffer, length, width, height, rotation, mirror);
}

void VideoRendererContext::setParameters(uint32_t params) {
    m_pVideoRenderer->setParameters(params);
}

uint32_t VideoRendererContext::getParameters() {
    return m_pVideoRenderer->getParameters();
}

void VideoRendererContext::createContext(JNIEnv *env, jobject obj, jint type) {
    auto *context = new VideoRendererContext(type);

    storeContext(env, obj, context);
}

void VideoRendererContext::storeContext(JNIEnv *env, jobject obj, VideoRendererContext *context) {
    // Get a reference to this object's class
    jclass cls = env->GetObjectClass(obj);

    if (nullptr == cls) {
        LOGE("Could not find com/media/camera/preview/render/VideoRenderer.");
        return;
    }

    // Get the Field ID of the "mNativeContext" variable
    jni_fields.context = env->GetFieldID(cls, "mNativeContext", "J");
    if (nullptr == jni_fields.context) {
        LOGE("Could not find mNativeContext.");
        return;
    }

    env->SetLongField(obj, jni_fields.context, (jlong) context);
}

void VideoRendererContext::deleteContext(JNIEnv *env, jobject obj) {
    if (nullptr == jni_fields.context) {
        LOGE("Could not find mNativeContext.");
        return;
    }

    auto *context = reinterpret_cast<VideoRendererContext *>(env->GetLongField(obj, jni_fields.context));

    delete context;

    env->SetLongField(obj, jni_fields.context, 0L);
}

VideoRendererContext *VideoRendererContext::getContext(JNIEnv *env, jobject obj) {
    if (nullptr == jni_fields.context) {
        LOGE("Could not find mNativeContext.");
        return nullptr;
    }

    auto *context = reinterpret_cast<VideoRendererContext *>(env->GetLongField(obj, jni_fields.context));

    return context;
}
