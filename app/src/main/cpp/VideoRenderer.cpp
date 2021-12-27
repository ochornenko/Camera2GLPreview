#include "VideoRenderer.h"
#include "GLVideoRendererYUV420.h"
#include "VKVideoRendererYUV420.h"
#include "GLVideoRendererYUV420Filter.h"

VideoRenderer::VideoRenderer()
        : m_width(0),
          m_height(0),
          m_backingWidth(0),
          m_backingHeight(0),
          m_params(0),
          m_rotation(0),
          isDirty(false),
          isProgramChanged(false) {

}

VideoRenderer::~VideoRenderer() = default;

std::unique_ptr<VideoRenderer> VideoRenderer::create(int type) {
    switch (type) {
        case tYUV420_FILTER:
            return std::unique_ptr<VideoRenderer>(std::make_unique<GLVideoRendererYUV420Filter>());
        case tVK_YUV420:
            return std::unique_ptr<VideoRenderer>(std::make_unique<VKVideoRendererYUV420>());
        case tYUV420:
        default:
            return std::unique_ptr<VideoRenderer>(std::make_unique<GLVideoRendererYUV420>());
    }
}
