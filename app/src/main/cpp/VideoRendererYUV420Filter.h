#ifndef _H_VIDEO_RENDER_YUV_FILTER_
#define _H_VIDEO_RENDER_YUV_FILTER_

#include "VideoRendererYUV420.h"
#include <vector>

class VideoRendererYUV420Filter : public VideoRendererYUV420 {

public:
    VideoRendererYUV420Filter();
    virtual ~VideoRendererYUV420Filter();

    virtual void render() override ;
    virtual void applyFilter(int filter) override;
    virtual int getMaxFilter() override;

private:
    size_t m_filter = 0;
    size_t m_prevFilter = 0;

    std::vector<const char *> m_fragmentShader;
};

#endif //_H_VIDEO_RENDER_YUV_FILTER_
