#include "GLUtils.h"

#include <stdlib.h>
#include <cmath>

void check_gl_error(const char *op)
{
    for (GLint error = glGetError(); error; error = glGetError())
    {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

GLuint load_shader(GLenum shaderType, const char *pSource)
{
	GLuint shader = glCreateShader(shaderType);
    if (shader)
    {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char* buf = (char*) malloc((size_t)infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint create_program(const char *pVertexSource, const char *pFragmentSource, GLuint &vertexShader,
                      GLuint &pixelShader)
{
    vertexShader = load_shader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) return 0;

    pixelShader = load_shader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) return 0;

    GLuint program = glCreateProgram();
    if (program)
    {
        glAttachShader(program, vertexShader);
        check_gl_error("glAttachShader");
        glAttachShader(program, pixelShader);
        check_gl_error("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

        glDetachShader(program, vertexShader);
        glDeleteShader(vertexShader);
        vertexShader = 0;
        glDetachShader(program, pixelShader);
        glDeleteShader(pixelShader);
        pixelShader = 0;
        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char* buf = (char*) malloc((size_t)bufLength);
                if (buf)
                {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }

            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

void delete_program(GLuint &program)
{
    if (program)
    {
        glUseProgram(0);
        glDeleteProgram(program);
        program = 0;
    }
}

void mat4f_load_ortho(float left, float right, float bottom, float top, float near, float far, float* mat4f)
{
	float r_l = right - left;
	float t_b = top - bottom;
	float f_n = far - near;
	float tx = - (right + left) / (right - left);
	float ty = - (top + bottom) / (top - bottom);
	float tz = - (far + near) / (far - near);

	mat4f[0] = 2.0f / r_l;
	mat4f[1] = 0.0f;
	mat4f[2] = 0.0f;
	mat4f[3] = 0.0f;

	mat4f[4] = 0.0f;
	mat4f[5] = 2.0f / t_b;
	mat4f[6] = 0.0f;
	mat4f[7] = 0.0f;

	mat4f[8] = 0.0f;
	mat4f[9] = 0.0f;
	mat4f[10] = -2.0f / f_n;
	mat4f[11] = 0.0f;

	mat4f[12] = tx;
	mat4f[13] = ty;
	mat4f[14] = tz;
	mat4f[15] = 1.0f;
}

void mat4f_load_rotation_z(int rotation, float* mat4f)
{
    float radians = rotation * (float)M_PI / 180.0f;
    float s = std::sin(radians);
    float c = std::cos(radians);

    mat4f[0] = c;
    mat4f[1] = -s;
    mat4f[2] = 0.0f;
    mat4f[3] = 0.0f;

    mat4f[4] = s;
    mat4f[5] = c;
    mat4f[6] = 0.0f;
    mat4f[7] = 0.0f;

    mat4f[8] = 0.0f;
    mat4f[9] = 0.0f;
    mat4f[10] = 1.0f;
    mat4f[11] = 0.0f;

    mat4f[12] = 0.0f;
    mat4f[13] = 0.0f;
    mat4f[14] = 0.0f;
    mat4f[15] = 1.0f;
}

void mat4f_load_scale(float scaleX, float scaleY, float scaleZ, float* mat4f)
{
    mat4f[0] = scaleX;
    mat4f[1] = 0.0f;
    mat4f[2] = 0.0f;
    mat4f[3] = 0.0f;

    mat4f[4] = 0.0f;
    mat4f[5] = scaleY;
    mat4f[6] = 0.0f;
    mat4f[7] = 0.0f;

    mat4f[8] = 0.0f;
    mat4f[9] = 0.0f;
    mat4f[10] = scaleZ;
    mat4f[11] = 0.0f;

    mat4f[12] = 0.0f;
    mat4f[13] = 0.0f;
    mat4f[14] = 0.0f;
    mat4f[15] = 1.0f;
}

float aspect_ratio_correction(bool fillScreen, size_t backingWidth, size_t backingHeight, size_t width, size_t height)
{
    float backingAspectRatio = (float)backingWidth / (float)backingHeight;
    float targetAspectRatio = (float)width / (float)height;

    float scalingFactor;
    if (fillScreen)
    {
        if (backingAspectRatio > targetAspectRatio)
        {
            scalingFactor = (float)backingWidth / (float)width;
        }
        else
        {
            scalingFactor = (float)backingHeight / (float)height;
        }
    }
    else
    {
        if (backingAspectRatio > targetAspectRatio)
        {
            scalingFactor =  (float)backingHeight / (float)height;
        }
        else
        {
            scalingFactor = (float)backingWidth / (float)width;
        }
    }

    return scalingFactor;
}
