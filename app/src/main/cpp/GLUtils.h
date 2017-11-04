#ifndef _H_GL_UTILS_
#define _H_GL_UTILS_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>

#define  LOG_TAG "libmedia"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

GLuint load_shader(GLenum shaderType, const char *pSource);
GLuint create_program(const char *pVertexSource, const char *pFragmentSource, GLuint &vertexShader,
                      GLuint &pixelShader);
void check_gl_error(const char *op);
void mat4f_load_ortho(float left, float right, float bottom, float top, float near, float far, float* mout);

#endif // _H_GL_UTILS_
