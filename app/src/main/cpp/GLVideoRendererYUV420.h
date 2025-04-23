#ifndef _GL_VIDEO_RENDERER_YUV_H_
#define _GL_VIDEO_RENDERER_YUV_H_

#include "VideoRenderer.h"
#include "GLUtils.h"

class GLVideoRendererYUV420 : public VideoRenderer {
public:
    GLVideoRendererYUV420();

    ~GLVideoRendererYUV420() override;

    void
    init(ANativeWindow *window, AAssetManager *assetManager, size_t width, size_t height) override;

    void render() override;

    void draw(uint8_t *buffer, size_t length, size_t width, size_t height, float rotation, bool mirror) override;

    void setParameters(uint32_t params) override;

    uint32_t getParameters() override;

    int createProgram(const char *pVertexSource, const char *pFragmentSource) override;

protected:
    virtual GLuint useProgram();

    GLuint m_program;
    GLuint m_vertexShader;
    GLuint m_pixelShader;
private:
    bool createTextures();

    bool updateTextures();

    void deleteTextures();

    void updateFrame(const video_frame &frame);

    std::unique_ptr<uint8_t[]> m_pDataY;

    uint8_t *m_pDataU;
    uint8_t *m_pDataV;

    size_t m_sizeY;
    size_t m_sizeU;
    size_t m_sizeV;

    GLuint m_textureIdY;
    GLuint m_textureIdU;
    GLuint m_textureIdV;

    GLuint m_vertexPos;
    GLint m_rotationLoc;
    GLint m_scaleLoc;
    GLuint m_textureLoc;
    GLint m_textureYLoc;
    GLint m_textureULoc;
    GLint m_textureVLoc;
    GLint m_textureSize;
};

#endif //_GL_VIDEO_RENDERER_YUV_H_
