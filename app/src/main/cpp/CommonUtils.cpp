#include "CommonUtils.h"

#include <cmath>

void load_identity(float *m) {
    for (int i = 0; i < 16; ++i) {
        m[i] = 0.0f;
    }
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void mat4f_load_rotate_mat(float *m, float rotation) {
    load_identity(m);

    float radians = rotation * (float) M_PI / 180.0f;
    float c = cosf(radians);
    float s = sinf(radians);

    m[0] = c;
    m[1] = s;
    m[4] = -s;
    m[5] = c;
}

void mat4f_load_scale_mat(float *m, int rotation, size_t surfaceWidth, size_t surfaceHeight,
                          size_t frameWidth, size_t frameHeight, bool mirrorX, bool mirrorY) {
    auto rotatedFrameWidth = frameHeight;
    auto rotatedFrameHeight = frameWidth;

    if (rotation % 180 == 0) {
        rotatedFrameWidth = frameWidth;
        rotatedFrameHeight = frameHeight;
    }

    float surfaceAspectRatio = (float) surfaceWidth / (float) surfaceHeight;
    float frameAspectRatio = (float) rotatedFrameWidth / (float) rotatedFrameHeight;

    float scaleX = 1.0f;
    float scaleY = 1.0f;

    if (frameAspectRatio > surfaceAspectRatio) {
        scaleX = surfaceAspectRatio / frameAspectRatio;
    } else {
        scaleY = frameAspectRatio / surfaceAspectRatio;
    }

    load_identity(m);

    m[0] = mirrorX ? scaleX : -scaleX;
    m[5] = mirrorY ? scaleY : -scaleY;
}
