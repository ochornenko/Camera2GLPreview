#ifndef _H_VIDEO_RENDERER_
#define _H_VIDEO_RENDERER_

#include <cstdint>
#include <memory>
#include <android/native_window.h>
#include <android/asset_manager.h>

enum {
    tYUV420, tVK_YUV420, tYUV420_FILTER
};

struct video_frame {
    size_t width;
    size_t height;
    size_t stride_y;
    size_t stride_uv;
    uint8_t *y;
    uint8_t *u;
    uint8_t *v;
};

class VideoRenderer {
public:
    VideoRenderer();

    virtual ~VideoRenderer();

    static std::unique_ptr<VideoRenderer> create(int type);

    virtual void init(ANativeWindow *window, AAssetManager *assetManager, size_t width, size_t height) = 0;

    virtual void render() = 0;

    virtual void updateFrame(const video_frame &frame) = 0;

    virtual void
    draw(uint8_t *buffer, size_t length, size_t width, size_t height, float rotation) = 0;

    virtual void setParameters(uint32_t params) = 0;

    virtual uint32_t getParameters() = 0;

    virtual int createProgram(const char *pVertexSource, const char *pFragmentSource) = 0;

protected:
    size_t m_width;
    size_t m_height;
    size_t m_backingWidth;
    size_t m_backingHeight;
    uint32_t m_params;
    float m_rotation;

    bool isDirty;
    bool isProgramChanged;
};

#endif // _H_VIDEO_RENDERER_
