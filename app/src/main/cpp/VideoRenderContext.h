#ifndef _H_VIDEO_RENDER_CONTEXT_
#define _H_VIDEO_RENDER_CONTEXT_

#include "VideoRender.h"

#include <memory>
#include <jni.h>

class VideoRenderContext
{
public:
	struct jni_fields_t
	{
		jfieldID context;
	};

	VideoRenderContext();
	~VideoRenderContext();

	void render();
	void draw(uint8_t *buffer, size_t length, size_t width, size_t height);

	static void createContext(JNIEnv *env, jobject obj);
	static void storeContext(JNIEnv *env, jobject obj, VideoRenderContext *context);
	static void deleteContext(JNIEnv *env, jobject obj);
	static VideoRenderContext* getContext(JNIEnv *env, jobject obj);

private:
    std::unique_ptr<VideoRender> m_pVideoRender;

	static jni_fields_t jni_fields;
};

#endif // _H_VIDEO_RENDER_CONTEXT_
