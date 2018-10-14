#include "VideoRenderer.h"
#include "VideoRendererYUV420.h"
#include "VideoRendererYUV420Filter.h"

VideoRenderer::VideoRenderer()
	: m_program(0)
	, m_vertexShader(0)
	, m_pixelShader(0)
	, m_width(0)
	, m_height(0)
	, m_backingWidth(0)
	, m_backingHeight(0)
	, isDirty(false)
	, isProgramChanged(false)
{

}

VideoRenderer::~VideoRenderer()
{
    delete_program(m_program);
}

std::unique_ptr<VideoRenderer> VideoRenderer::create(int type)
{
	switch(type)
    {
		case tYUV420_FILTER:
			return std::unique_ptr<VideoRenderer>(std::make_unique<VideoRendererYUV420Filter>());
		case tYUV420:
		default:
			return std::unique_ptr<VideoRenderer>(std::make_unique<VideoRendererYUV420>());
	}
}
