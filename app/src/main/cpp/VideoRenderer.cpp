#include "VideoRenderer.h"
#include "GLVideoRendererYUV420.h"
#include "VKVideoRendererYUV420.h"
#include "GLVideoRendererYUV420Filter.h"

VideoRenderer::VideoRenderer()
        : m_frameWidth(0),
          m_frameHeight(0),
          m_surfaceWidth(0),
          m_surfaceHeight(0),
          m_params(0),
          m_rotation(0),
          m_mirror(true),
          isDirty(false),
          isProgramChanged(false) {

}

VideoRenderer::~VideoRenderer() = default;

std::unique_ptr<VideoRenderer> VideoRenderer::create(int type) {
    switch (type) {
        case tYUV420_FILTER:
            return {std::make_unique<GLVideoRendererYUV420Filter>()};
        case tVK_YUV420:
            return {std::make_unique<VKVideoRendererYUV420>()};
        case tYUV420:
        default:
            return {std::make_unique<GLVideoRendererYUV420>()};
    }
}
