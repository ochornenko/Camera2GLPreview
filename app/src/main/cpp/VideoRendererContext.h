#ifndef _H_VIDEO_RENDERER_CONTEXT_
#define _H_VIDEO_RENDERER_CONTEXT_

#include "VideoRenderer.h"

#include <memory>
#include <jni.h>

class VideoRendererContext
{
public:
	struct jni_fields_t
	{
		jfieldID context;
	};

	VideoRendererContext();
	~VideoRendererContext();

    void init(size_t width, size_t height);
	void render();
	void draw(uint8_t *buffer, size_t length, size_t width, size_t height, int rotation);
	void applyFilter(int filter);
	int getMaxFilter();

	static void createContext(JNIEnv *env, jobject obj);
	static void storeContext(JNIEnv *env, jobject obj, VideoRendererContext *context);
	static void deleteContext(JNIEnv *env, jobject obj);
	static VideoRendererContext* getContext(JNIEnv *env, jobject obj);

private:
    std::unique_ptr<VideoRenderer> m_pVideoRenderer;

	static jni_fields_t jni_fields;
};

#endif // _H_VIDEO_RENDERER_CONTEXT_
