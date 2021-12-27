#ifndef _H_GL_UTILS_
#define _H_GL_UTILS_

#include <GLES3/gl3.h>

GLuint load_shader(GLenum shaderType, const char *pSource);

GLuint create_program(const char *pVertexSource, const char *pFragmentSource, GLuint &vertexShader,
                      GLuint &pixelShader);

void delete_program(GLuint &program);

void check_gl_error(const char *op);

#endif // _H_GL_UTILS_
