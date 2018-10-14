#include "VideoRendererYUV420.h"
#include "GLShaders.h"

// Vertices for a full screen quad.
static const float kVertices[8] = {
  -1.f, 1.f,
  -1.f, -1.f,
  1.f, 1.f,
  1.f, -1.f
};

// Texture coordinates for mapping entire texture.
static const float kTextureCoords[8] = {
  0, 0,
  0, 1,
  1, 0,
  1, 1
};

VideoRendererYUV420::VideoRendererYUV420()
    : m_pDataY(nullptr)
	, m_pDataU(nullptr)
	, m_pDataV(nullptr)
    , m_length(0)
	, m_sizeY(0)
	, m_sizeU(0)
	, m_sizeV(0)
	, m_textureIdY(0)
	, m_textureIdU(0)
	, m_textureIdV(0)
	, m_vertexPos(0)
    , m_textureLoc(0)
	, m_textureYLoc(0)
	, m_textureULoc(0)
	, m_textureVLoc(0)
	, m_uniformProjection(0)
    , m_uniformRotation(0)
    , m_uniformScale(0)
    , m_rotation(0)

{
	isProgramChanged = true;
}

VideoRendererYUV420::~VideoRendererYUV420()
{
	deleteTextures();
	delete_program(m_program);
}

void VideoRendererYUV420::init(size_t width, size_t height)
{
    m_backingWidth = width;
    m_backingHeight = height;
}

void VideoRendererYUV420::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if (!updateTextures() || !useProgram()) return;

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void VideoRendererYUV420::updateFrame(const video_frame& frame)
{
	m_sizeY = frame.width * frame.height;
	m_sizeU = frame.width * frame.height / 4;
	m_sizeV = frame.width * frame.height / 4;

	if (m_pDataY == nullptr || m_width != frame.width || m_height != frame.height)
    {
        m_pDataY = std::make_unique<uint8_t[]>(m_sizeY + m_sizeU + m_sizeV);
		m_pDataU = m_pDataY.get() + m_sizeY;
		m_pDataV = m_pDataU + m_sizeU;
	}

	m_width = frame.width;
	m_height = frame.height;

	if (m_width == frame.stride_y)
    {
		memcpy(m_pDataY.get(), frame.y, m_sizeY);
	}
    else
    {
		uint8_t* pSrcY = frame.y;
		uint8_t* pDstY = m_pDataY.get();

		for (int h = 0; h < m_height; h++)
        {
			memcpy(pDstY, pSrcY, m_width);

			pSrcY += frame.stride_y;
			pDstY += m_width;
		}
	}

	if (m_width / 2 == frame.stride_uv)
    {
		memcpy(m_pDataU, frame.u, m_sizeU);
		memcpy(m_pDataV, frame.v, m_sizeV);
	}
    else
    {
		uint8_t* pSrcU = frame.u;
		uint8_t* pSrcV = frame.v;
		uint8_t* pDstU = m_pDataU;
		uint8_t* pDstV = m_pDataV;

		for (int h = 0; h < m_height / 2; h++)
        {
			memcpy(pDstU, pSrcU, m_width / 2);
			memcpy(pDstV, pSrcV, m_width / 2);

			pDstU += m_width / 2;
			pDstV += m_width / 2;

			pSrcU += frame.stride_uv;
			pSrcV += frame.stride_uv;
		}
	}

	isDirty = true;
}

void VideoRendererYUV420::draw(uint8_t *buffer, size_t length, size_t width, size_t height, int rotation)
{
    m_length = length;
    m_rotation = rotation;

	video_frame frame;
	frame.width = width;
	frame.height = height;
	frame.stride_y = width;
	frame.stride_uv = width / 2;
	frame.y = buffer;
	frame.u = buffer + width * height;
	frame.v = buffer + width * height * 5 / 4;

	updateFrame(frame);
}

void VideoRendererYUV420::applyFilter(int filter)
{
}

int VideoRendererYUV420::getMaxFilter()
{
    return 0;
}

bool VideoRendererYUV420::createTextures()
{
    GLsizei widthY = (GLsizei)m_width;
    GLsizei heightY = (GLsizei)m_height;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_textureIdY);
	glBindTexture(GL_TEXTURE_2D, m_textureIdY);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widthY, heightY, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    if (!m_textureIdY)
    {
        check_gl_error("Create Y texture");
        return false;
    }

	GLsizei widthU = (GLsizei)m_width / 2;
    GLsizei heightU = (GLsizei)m_height / 2;

	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_textureIdU);
	glBindTexture(GL_TEXTURE_2D, m_textureIdU);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widthU, heightU, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    if (!m_textureIdU)
    {
        check_gl_error("Create U texture");
        return false;
    }

    GLsizei widthV = (GLsizei)m_width / 2;
    GLsizei heightV = (GLsizei)m_height / 2;

	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &m_textureIdV);
	glBindTexture(GL_TEXTURE_2D, m_textureIdV);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widthV, heightV, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    if (!m_textureIdV)
    {
        check_gl_error("Create V texture");
        return false;
    }

	return true;
}

bool VideoRendererYUV420::updateTextures()
{
	if (!m_textureIdY && !m_textureIdU && !m_textureIdV && !createTextures()) return false;

	if (isDirty)
    {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureIdY);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)m_width, (GLsizei)m_height, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pDataY.get());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_textureIdU);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)m_width / 2, (GLsizei)m_height / 2, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pDataU);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_textureIdV);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)m_width / 2, (GLsizei)m_height / 2, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pDataV);

        isDirty = false;

        return true;
	}

	return false;
}

void VideoRendererYUV420::deleteTextures()
{
	if (m_textureIdY)
    {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_textureIdY);

		m_textureIdY = 0;
	}

	if (m_textureIdU)
    {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_textureIdU);

		m_textureIdU = 0;
	}

	if (m_textureIdV)
    {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_textureIdV);

		m_textureIdV = 0;
	}
}

GLuint VideoRendererYUV420::createProgram(const char *pVertexSource, const char *pFragmentSource)
{
	m_program = create_program(pVertexSource, pFragmentSource, m_vertexShader, m_pixelShader);

	if (!m_program)
    {
        check_gl_error("Create program");
		LOGE("Could not create program.");
        return 0;
	}

	m_vertexPos = (GLuint)glGetAttribLocation(m_program, "position");
	m_uniformProjection = glGetUniformLocation(m_program, "projection");
    m_uniformRotation = glGetUniformLocation(m_program, "rotation");
    m_uniformScale = glGetUniformLocation(m_program, "scale");
	m_textureYLoc = glGetUniformLocation(m_program, "s_textureY");
	m_textureULoc = glGetUniformLocation(m_program, "s_textureU");
	m_textureVLoc = glGetUniformLocation(m_program, "s_textureV");
	m_textureLoc = (GLuint)glGetAttribLocation(m_program, "texcoord");

	return m_program;
}

GLuint VideoRendererYUV420::useProgram()
{
	if (!m_program && !createProgram(kVertexShader, kFragmentShader))
    {
		LOGE("Could not use program.");
		return 0;
	}

	if (isProgramChanged)
    {
		glUseProgram(m_program);

        check_gl_error("Use program.");

		glVertexAttribPointer(m_vertexPos, 2, GL_FLOAT, GL_FALSE, 0, kVertices);
		glEnableVertexAttribArray(m_vertexPos);

        float targetAspectRatio = (float)m_height / (float)m_width;

		GLfloat projection[16];
        mat4f_load_ortho(-targetAspectRatio, targetAspectRatio, -1.0f, 1.0f, -1.0f, 1.0f, projection);
		glUniformMatrix4fv(m_uniformProjection, 1, GL_FALSE, projection);

        GLfloat rotationZ[16];
        mat4f_load_rotation_z(m_rotation, rotationZ);
        glUniformMatrix4fv(m_uniformRotation, 1, 0, &rotationZ[0]);

        float scaleFactor = aspect_ratio_correction(false, m_backingWidth, m_backingHeight, m_width, m_height);

        GLfloat scale[16];
        mat4f_load_scale(scaleFactor, scaleFactor, 1.0f, scale);
        glUniformMatrix4fv(m_uniformScale, 1, 0, &scale[0]);

		glUniform1i(m_textureYLoc, 0);
		glUniform1i(m_textureULoc, 1);
		glUniform1i(m_textureVLoc, 2);
		glVertexAttribPointer(m_textureLoc, 2, GL_FLOAT, GL_FALSE, 0, kTextureCoords);
		glEnableVertexAttribArray(m_textureLoc);

		isProgramChanged = false;
	}

	return m_program;
}
