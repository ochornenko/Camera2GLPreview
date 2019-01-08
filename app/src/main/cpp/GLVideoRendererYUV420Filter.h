#ifndef _GL_VIDEO_RENDERER_YUV_FILTER_H_
#define _GL_VIDEO_RENDERER_YUV_FILTER_H_

#include "GLVideoRendererYUV420.h"
#include <vector>

class GLVideoRendererYUV420Filter : public GLVideoRendererYUV420 {

public:
    GLVideoRendererYUV420Filter();
    virtual ~GLVideoRendererYUV420Filter();

    virtual void render() override ;
    virtual void setParameters(uint32_t params) override;
    virtual uint32_t getParameters() override;

private:
    size_t m_filter = 0;
    size_t m_prevFilter = 0;

    std::vector<const char *> m_fragmentShader;
};

#endif //_GL_VIDEO_RENDERER_YUV_FILTER_H_
