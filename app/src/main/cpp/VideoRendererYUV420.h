#ifndef _H_VIDEO_RENDER_YUV_
#define _H_VIDEO_RENDER_YUV_

#include "VideoRenderer.h"

class VideoRendererYUV420 : public VideoRenderer
{
public:
	VideoRendererYUV420();
	virtual ~VideoRendererYUV420();

    virtual void init(size_t width, size_t height) override;
	virtual void render() override;
	virtual void updateFrame(const video_frame& frame) override;
	virtual void draw(uint8_t *buffer, size_t length, size_t width, size_t height, int rotation) override;
	virtual bool createTextures() override;
	virtual bool updateTextures() override;
	virtual void deleteTextures() override;
	virtual GLuint createProgram() override;
	virtual GLuint useProgram() override;

private:
    int m_rotation;
    size_t m_length;

	std::unique_ptr<uint8_t[]> m_pDataY;
    std::unique_ptr<uint8_t[]> m_pDataU;
    std::unique_ptr<uint8_t[]> m_pDataV;

	size_t m_sizeY;
    size_t m_sizeU;
    size_t m_sizeV;

	GLuint m_texIdY;
	GLuint m_texIdU;
	GLuint m_texIdV;

	GLuint m_vertexPos;
    GLuint m_texLoc;
	GLint m_texYLoc;
	GLint m_texULoc;
	GLint m_texVLoc;
	GLint m_uniformMatrix;
};

#endif // _H_VIDEO_RENDER_YUV_
