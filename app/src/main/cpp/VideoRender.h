#ifndef _H_VIDEO_RENDER_
#define _H_VIDEO_RENDER_

#include "GLUtils.h"

#include <stdint.h>
#include <memory>

enum { tYUV420 };

struct video_frame
{
	size_t width;
	size_t height;
	size_t stride_y;
	size_t stride_uv;
	uint8_t* y;
	uint8_t* u;
	uint8_t* v;
};

class VideoRender {
public:
	VideoRender();
	virtual ~VideoRender();

	virtual void render() = 0;
	virtual void updateFrame(const video_frame& frame) = 0;
	virtual void draw(uint8_t *buffer, size_t length, size_t width, size_t height) = 0;
	virtual bool createTextures() = 0;
	virtual bool updateTextures() = 0;
	virtual void deleteTextures() = 0;
	virtual GLuint createProgram() = 0;
	virtual GLuint useProgram() = 0;

	static std::unique_ptr<VideoRender> create(int type);

protected:
	GLuint m_program;
	GLuint m_vertexShader;
	GLuint m_pixelShader;

	size_t m_width;
	size_t m_height;

	bool isDataChanged;
	bool isOrientationChanged;
};

#endif // _H_VIDEO_RENDER_
