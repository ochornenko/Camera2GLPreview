#include "VideoRenderYUV420.h"

#include <cmath>

// Vertices for a full screen quad.
static const float kVertices[8] = {
  -1.f, 1.f,
  -1.f, -1.f,
  1.f, 1.f,
  1.f, -1.f,
};

// Texture Coordinates mapping the entire texture.
static const float kTextureCoords[8] = {
  0, 0,
  0, 1,
  1, 0,
  1, 1,
};

// Pass-through vertex shader.
static const char kVertexShader[] =
    "varying vec2 interp_tc;\n"
    "\n"
    "attribute vec4 in_pos;\n"
    "attribute vec2 in_tc;\n"
	"uniform mat4 projection;\n"
    "uniform mat4 model_view;\n"
    "\n"
    "void main() {\n"
    "  interp_tc = in_tc;\n"
    "  gl_Position = projection * model_view * in_pos;\n"
    "}\n";

// YUV to RGB pixel shader. Loads a pixel from each plane and pass through the
// matrix.
static const char kFragmentShader[] =
    "precision highp float;\n"
	"varying vec2 interp_tc;\n"
	"uniform sampler2D y_tex;\n"
	"uniform sampler2D u_tex;\n"
	"uniform sampler2D v_tex;\n"
    "void main() {\n"
	"  float y = texture2D(y_tex, interp_tc).r;\n"
    "  float u = texture2D(u_tex, interp_tc).r - .5;\n"
    "  float v = texture2D(v_tex, interp_tc).r - .5;\n"
	"  float r = y +             1.402 * v;\n"
	"  float g = y - 0.344 * u - 0.714 * v;\n"
	"  float b = y + 1.772 * u;\n"
	"  gl_FragColor = vec4(r, g, b, 1.0);\n"
    "}\n";

VideoRenderYUV420::VideoRenderYUV420()
    : m_length(0)
	, m_pDataY(nullptr)
	, m_pDataU(nullptr)
	, m_pDataV(nullptr)
	, m_sizeY(0)
	, m_sizeU(0)
	, m_sizeV(0)
	, m_texIdY(0)
	, m_texIdU(0)
	, m_texIdV(0)
	, m_vertexPos(0)
    , m_texLoc(0)
	, m_texYLoc(0)
	, m_texULoc(0)
	, m_texVLoc(0)
	, m_uniformMatrix(0)
{
	isOrientationChanged = true;
}

VideoRenderYUV420::~VideoRenderYUV420()
{
	deleteTextures();
}

void VideoRenderYUV420::render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if (!updateTextures() || !useProgram()) return;

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    check_gl_error("glDrawArrays");
}

void VideoRenderYUV420::updateFrame(const video_frame& frame)
{
	m_sizeY = frame.width * frame.height;
	m_sizeU = frame.width * frame.height / 4;
	m_sizeV = frame.width * frame.height / 4;

	if (m_pDataY == nullptr || m_width != frame.width || m_height != frame.height)
    {
        m_pDataY = std::make_unique<uint8_t[]>(m_sizeY + m_sizeU + m_sizeV);
		m_pDataU = std::unique_ptr<uint8_t[]>(m_pDataY.get() + m_sizeY);
		m_pDataV = std::unique_ptr<uint8_t[]>(m_pDataU.get() + m_sizeU);
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
		memcpy(m_pDataU.get(), frame.u, m_sizeU);
		memcpy(m_pDataV.get(), frame.v, m_sizeV);
	}
    else
    {
		uint8_t* pSrcU = frame.u;
		uint8_t* pSrcV = frame.v;
		uint8_t* pDstU = m_pDataU.get();
		uint8_t* pDstV = m_pDataV.get();

		for (int h = 0; h < m_height / 2; h++)
        {
			memcpy(pDstU, pSrcU, m_width/2);
			memcpy(pDstV, pSrcV, m_width/2);

			pDstU += m_width/2;
			pDstV += m_width/2;

			pSrcU += frame.stride_uv;
			pSrcV += frame.stride_uv;
		}
	}

	isDataChanged = true;
}

void VideoRenderYUV420::draw(uint8_t *buffer, size_t length, size_t width, size_t height)
{
    m_length = length;

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

bool VideoRenderYUV420::createTextures()
{
    GLsizei widthY = (GLsizei)m_width;
    GLsizei heightY = (GLsizei)m_height;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_texIdY);
	glBindTexture(GL_TEXTURE_2D, m_texIdY);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widthY, heightY, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    check_gl_error("Create Y texture");

	GLsizei widthU = (GLsizei)m_width / 2;
    GLsizei heightU = (GLsizei)m_height / 2;

	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_texIdU);
	glBindTexture(GL_TEXTURE_2D, m_texIdU);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widthU, heightU, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    check_gl_error("Create U texture");

    GLsizei widthV = (GLsizei)m_width / 2;
    GLsizei heightV = (GLsizei)m_height / 2;

	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &m_texIdV);
	glBindTexture(GL_TEXTURE_2D, m_texIdV);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widthV, heightV, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    check_gl_error("Create V texture");

	return true;
}

bool VideoRenderYUV420::updateTextures()
{
	if (!m_texIdY && !createTextures()) return false;

	if (isDataChanged)
    {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texIdY);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)m_width, (GLsizei)m_height, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pDataY.get());

        check_gl_error("Update Y texture");

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_texIdU);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)m_width / 2, (GLsizei)m_height / 2, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pDataU.get());

        check_gl_error("Update U texture");

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_texIdV);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)m_width / 2, (GLsizei)m_height / 2, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pDataV.get());

        check_gl_error("Update V texture");
	}

	isDataChanged = false;

	return true;
}

void VideoRenderYUV420::deleteTextures()
{
	if (m_texIdY)
    {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_texIdY);

        check_gl_error("Delete Y texture");

		m_texIdY = 0;
	}

	if (m_texIdU)
    {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_texIdU);

        check_gl_error("Delete U texture");

		m_texIdU = 0;
	}

	if (m_texIdV)
    {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_texIdV);

        check_gl_error("Delete V texture");

		m_texIdV = 0;
	}
}

GLuint VideoRenderYUV420::createProgram()
{
	m_program = ::create_program(kVertexShader, kFragmentShader, m_vertexShader, m_pixelShader);
	if (!m_program)
    {
        check_gl_error("Create program");
		LOGE("Could not create program.");
	}

	m_vertexPos = (GLuint)glGetAttribLocation(m_program, "in_pos");
    check_gl_error("glGetAttribLocation");

	m_uniformMatrix = glGetUniformLocation(m_program, "projection");
    check_gl_error("glGetUniformLocation modelviewProj");

	m_texYLoc = glGetUniformLocation(m_program, "y_tex");
    check_gl_error("glGetUniformLocation y_tex");
	m_texULoc = glGetUniformLocation(m_program, "u_tex");
    check_gl_error("glGetUniformLocation u_tex");
	m_texVLoc = glGetUniformLocation(m_program, "v_tex");
    check_gl_error("glGetUniformLocation v_tex");

	m_texLoc = (GLuint)glGetAttribLocation(m_program, "in_tc");
    check_gl_error("glGetAttribLocation in_tc.");

	return m_program;
}

GLuint VideoRenderYUV420::useProgram()
{
	if (!m_program && !createProgram())
    {
		LOGE("Could not use program.");
		return 0;
	}

	if (isOrientationChanged)
    {
		glUseProgram(m_program);

        check_gl_error("Use program.");

		// Bind parameters.
		glVertexAttribPointer(m_vertexPos, 2, GL_FLOAT, GL_FALSE, 0, kVertices);
        check_gl_error("glVertexAttribPointer");
		glEnableVertexAttribArray(m_vertexPos);
        check_gl_error("glEnableVertexAttribArray");

		GLfloat projection[16];
        mat4f_load_ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, projection);
		glUniformMatrix4fv(m_uniformMatrix, 1, GL_FALSE, projection);

        applyRotationZ(90);

		glUniform1i(m_texYLoc, 0);
        check_gl_error("Set uniform y_tex.");

		glUniform1i(m_texULoc, 1);
        check_gl_error("Set uniform u_tex.");

		glUniform1i(m_texVLoc, 2);
        check_gl_error("Set uniform v_tex.");

		glVertexAttribPointer(m_texLoc, 2, GL_FLOAT, GL_FALSE, 0, kTextureCoords);
        check_gl_error("glVertexAttribPointer");
		glEnableVertexAttribArray(m_texLoc);
        check_gl_error("glEnableVertexAttribArray");

		isOrientationChanged = false;
	}

	return m_program;
}

void VideoRenderYUV420::applyRotationZ(float degrees)
{
    float radians = degrees * 3.14159f / 180.0f;
    float s = std::sin(radians);
    float c = std::cos(radians);
    float zRotation[16] = {
            c, -s, 0, 0,
            s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
    };

    GLint modelviewUniform = glGetUniformLocation(m_program, "model_view");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &zRotation[0]);
}
