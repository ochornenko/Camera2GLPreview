#include "VideoRenderer.h"
#include "VideoRendererYUV420.h"

VideoRenderer::VideoRenderer()
	: m_program(0)
	, m_vertexShader(0)
	, m_pixelShader(0)
	, m_width(0)
	, m_height(0)
	, isDirty(false)
	, isOrientationChanged(false)
{

}

VideoRenderer::~VideoRenderer()
{
	if (m_vertexShader)
    {
		glDetachShader(m_program, m_vertexShader);
		glDeleteShader(m_vertexShader);
		m_vertexShader = 0;

	}
	if (m_pixelShader)
    {
		glDetachShader(m_program, m_pixelShader);
		glDeleteShader(m_pixelShader);
		m_pixelShader = 0;

	}
	if (m_program)
    {
		glDeleteProgram(m_program);
		m_program = 0;
	}
}

std::unique_ptr<VideoRenderer> VideoRenderer::create(int type)
{
	switch(type)
    {
		case tYUV420:
		default:
			return std::unique_ptr<VideoRenderer>(std::make_unique<VideoRendererYUV420>());
	}
}
