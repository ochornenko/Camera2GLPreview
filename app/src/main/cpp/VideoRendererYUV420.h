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
	virtual void applyFilter(int filter) override;
    virtual int getMaxFilter() override;
	virtual bool createTextures() override;
	virtual bool updateTextures() override;
	virtual void deleteTextures() override;
	virtual GLuint createProgram(const char *pVertexSource, const char *pFragmentSource) override;
	virtual GLuint useProgram() override;

private:
	std::unique_ptr<uint8_t[]> m_pDataY;

    uint8_t * m_pDataU;
    uint8_t * m_pDataV;

    size_t m_length;
	size_t m_sizeY;
    size_t m_sizeU;
    size_t m_sizeV;

	GLuint m_textureIdY;
	GLuint m_textureIdU;
	GLuint m_textureIdV;

	GLuint m_vertexPos;
    GLuint m_textureLoc;
	GLint m_textureYLoc;
	GLint m_textureULoc;
	GLint m_textureVLoc;
	GLint m_uniformProjection;
	GLint m_uniformRotation;
    GLint m_uniformScale;

    int m_rotation;
};

#endif // _H_VIDEO_RENDER_YUV_
