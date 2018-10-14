#ifndef _H_VIDEO_RENDERER_
#define _H_VIDEO_RENDERER_

#include "GLUtils.h"

#include <stdint.h>
#include <memory>

enum { tYUV420, tYUV420_FILTER };

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

class VideoRenderer {
public:
	VideoRenderer();
	virtual ~VideoRenderer();

    static std::unique_ptr<VideoRenderer> create(int type);

    virtual void init(size_t width, size_t height) = 0;
	virtual void render() = 0;
	virtual void updateFrame(const video_frame& frame) = 0;
	virtual void draw(uint8_t *buffer, size_t length, size_t width, size_t height, int rotation) = 0;
	virtual void applyFilter(int filter) = 0;
	virtual int getMaxFilter() = 0;
	virtual bool createTextures() = 0;
	virtual bool updateTextures() = 0;
	virtual void deleteTextures() = 0;
	virtual GLuint createProgram(const char *pVertexSource, const char *pFragmentSource) = 0;
	virtual GLuint useProgram() = 0;

protected:
	GLuint m_program;
	GLuint m_vertexShader;
	GLuint m_pixelShader;

	size_t m_width;
	size_t m_height;
    size_t m_backingWidth;
    size_t m_backingHeight;

	bool isDirty;
	bool isProgramChanged;
};

#endif // _H_VIDEO_RENDERER_
