#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

#include <cstddef>

void load_identity(float *m);

void mat4f_load_rotate_mat(float *m, float rotation);

void mat4f_load_scale_mat(float *m, int rotation, size_t surfaceWidth, size_t surfaceHeight,
                          size_t frameWidth, size_t frameHeight, bool mirrorX, bool mirror);

#endif //_COMMON_UTILS_H_
