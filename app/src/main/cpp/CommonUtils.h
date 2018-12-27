#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

#include <cstddef>

void mat4f_load_ortho(float left, float right, float bottom, float top, float near, float far, float* mat4f);
void mat4f_load_rotation_z(int rotation, float* mat4f);
void mat4f_load_scale(float scaleX, float scaleY, float scaleZ, float* mat4f);
float aspect_ratio_correction(bool fillScreen, size_t backingWidth, size_t backingHeight, size_t width, size_t height);

#endif //_COMMON_UTILS_H_
