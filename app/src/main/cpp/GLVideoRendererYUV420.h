#ifndef _GL_VIDEO_RENDERER_YUV_H_
#define _GL_VIDEO_RENDERER_YUV_H_

#include "VideoRenderer.h"
#include "GLUtils.h"

class GLVideoRendererYUV420 : public VideoRenderer
{
public:
	GLVideoRendererYUV420();
	virtual ~GLVideoRendererYUV420();

    virtual void init(ANativeWindow* window, size_t width, size_t height) override;
	virtual void render() override;
	virtual void updateFrame(const video_frame& frame) override;
	virtual void draw(uint8_t *buffer, size_t length, size_t width, size_t height, int rotation) override;
	virtual void setParameters(uint32_t params) override;
    virtual uint32_t getParameters() override;
	virtual bool createTextures() override;
	virtual bool updateTextures() override;
	virtual void deleteTextures() override;
	virtual int createProgram(const char *pVertexSource, const char *pFragmentSource) override;

protected:
	virtual GLuint useProgram();

	GLuint m_program;
	GLuint m_vertexShader;
	GLuint m_pixelShader;
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
	GLint m_textureSize;
	GLint m_uniformProjection;
	GLint m_uniformRotation;
    GLint m_uniformScale;
};

#endif //_GL_VIDEO_RENDERER_YUV_H_
