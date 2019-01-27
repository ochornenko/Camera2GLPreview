#include "CommonUtils.h"

#include <cmath>

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
    float scalingFactor = 1.0f;

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

    return scalingFactor;
}
