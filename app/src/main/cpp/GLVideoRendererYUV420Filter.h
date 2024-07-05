#ifndef _GL_VIDEO_RENDERER_YUV_FILTER_H_
#define _GL_VIDEO_RENDERER_YUV_FILTER_H_

#include "GLVideoRendererYUV420.h"
#include <vector>

class GLVideoRendererYUV420Filter : public GLVideoRendererYUV420 {

public:
    GLVideoRendererYUV420Filter();

    ~GLVideoRendererYUV420Filter() override;

    void render() override;

    void setParameters(uint32_t params) override;

    uint32_t getParameters() override;

private:
    size_t m_filter = 0;
    size_t m_prevFilter = 0;

    std::vector<const char *> m_fragmentShader;
};

#endif //_GL_VIDEO_RENDERER_YUV_FILTER_H_
