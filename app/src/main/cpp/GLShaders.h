#ifndef _H_GL_SHADER_H_
#define _H_GL_SHADER_H_

// Vertex shader.
static const char kVertexShader[] =
    "#version 100\n\
    varying vec2 v_texcoord; \
    attribute vec4 position; \
    attribute vec2 texcoord; \
    uniform mat4 projection; \
    uniform mat4 rotation; \
    uniform mat4 scale; \
    void main() { \
        v_texcoord = texcoord; \
        gl_Position = projection * rotation * scale * position; \
    }";

// Pixel shader, YUV420 to RGB conversion.
static const char kFragmentShader[] =
    "#version 100\n \
    precision highp float; \
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    void main() {\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, v_texcoord).r;\
        u = texture2D(s_textureU, v_texcoord).r;\
        v = texture2D(s_textureV, v_texcoord).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        gl_FragColor = vec4(r, g, b, 1.0);\
    }";

// Blur Filter
static const char kFragmentShader1[] =
    "#version 100\n \
    precision highp float; \
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    vec4 YuvToRgb(vec2 uv) {\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, uv).r;\
        u = texture2D(s_textureU, uv).r;\
        v = texture2D(s_textureV, uv).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        return vec4(r, g, b, 1.0);\
    }\
    void main() {\
        vec4 sample0, sample1, sample2, sample3;\
        float blurStep = 0.5;\
        float step = blurStep / 100.0;\
        sample0 = YuvToRgb(vec2(v_texcoord.x - step, v_texcoord.y - step));\
        sample1 = YuvToRgb(vec2(v_texcoord.x + step, v_texcoord.y + step));\
        sample2 = YuvToRgb(vec2(v_texcoord.x + step, v_texcoord.y - step));\
        sample3 = YuvToRgb(vec2(v_texcoord.x - step, v_texcoord.y + step));\
        gl_FragColor = (sample0 + sample1 + sample2 + sample3) / 4.0;\
    }";

// Swirl Filter
static const char kFragmentShader2[] =
    "#version 100\n\
    precision highp float; \
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    void main() {\
        float radius = 200.0;\
        float angle = 0.8;\
        vec2 center = vec2(640/2, 480/2);\
        vec2 texSize = vec2(640, 480);\
        vec2 tc = v_texcoord * texSize;\
        tc -= center;\
        float dist = length(tc);\
        if (dist < radius) {\
            float percent = (radius - dist) / radius;\
            float theta = percent * percent * angle * 8.0;\
            float s = sin(theta);\
            float c = cos(theta);\
            tc = vec2(dot(tc, vec2(c, -s)), dot(tc, vec2(s, c)));\
        }\
        tc += center;\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, tc / texSize).r;\
        u = texture2D(s_textureU, tc / texSize).r;\
        v = texture2D(s_textureV, tc / texSize).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        gl_FragColor = vec4(r, g, b, 1.0);\
    }";

static const char kFragmentShader3[] =
    "#version 100\n\
    precision highp float;\
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    void main() {\
        float y, u, v, r, g, b;\
        vec2 cen = vec2(0.5, 0.5) - v_texcoord.xy;\
        vec2 mcen = -0.07 * log(length(cen)) * normalize(cen);\
        y = texture2D(s_textureY, v_texcoord.xy - mcen).r;\
        u = texture2D(s_textureU, v_texcoord.xy - mcen).r;\
        v = texture2D(s_textureV, v_texcoord.xy - mcen).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        gl_FragColor = vec4(r, g, b, 1.0);\
    }";

// Ripple Filter
static const char kFragmentShader4[] =
    "#version 100\n \
    precision highp float; \
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    vec4 YuvToRgb(vec2 uv) {\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, uv).r;\
        u = texture2D(s_textureU, uv).r;\
        v = texture2D(s_textureV, uv).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        return vec4(r, g, b, 1.0);\
    }\
    void main() {\
        float amplitude = 0.03;\
        float speed = 0.004;\
        float twirliness = 12.0;\
        float time = 1.0;\
        vec2 tc = v_texcoord;\
        vec2 p = -1.0 + 2.0 * tc;\
        float len = length(p);\
        vec2 uv = tc + (p / len) * cos(len * twirliness - time * speed) * amplitude;\
        gl_FragColor = YuvToRgb(uv);\
    }";

// Wave Filter
static const char kFragmentShader5[] =
    "#version 100\n \
    precision highp float; \
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    vec4 YuvToRgb(vec2 uv) {\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, uv).r;\
        u = texture2D(s_textureU, uv).r;\
        v = texture2D(s_textureV, uv).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        return vec4(r, g, b, 1.0);\
    }\
    void main() {\
        float amplitude = 0.02;\
        float xTwirl = 5.0;\
        float yTwirl = 10.0;\
        float speed = 0.0025;\
        float time = 1.0;\
        vec2 uv = v_texcoord;\
        uv.x = uv.x + cos(uv.y * yTwirl + time * speed) * amplitude;\
        uv.y = uv.y + sin(uv.x * xTwirl + time * speed) * amplitude;\
        gl_FragColor = YuvToRgb(uv);\
    }";

// Magnifying Glass Filter
static const char kFragmentShader6[] =
    "#version 100\n \
    precision highp float; \
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    vec4 YuvToRgb(vec2 uv) {\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, uv).r;\
        u = texture2D(s_textureU, uv).r;\
        v = texture2D(s_textureV, uv).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        return vec4(r, g, b, 1.0);\
    }\
    void main() {\
        float circleRadius = float(0.5);\
        float minZoom = 0.4;\
        float maxZoom = 0.6;\
        vec2 texSize = vec2(640.,480.);\
        vec2 center = vec2(texSize.x / 2.0, texSize.y / 2.0);\
        vec2 uv = v_texcoord;\
        uv.x *= (texSize.x / texSize.y);\
        vec2 realCenter = vec2(0.0, 0.0);\
        realCenter.x = (center.x / texSize.x) * (texSize.x / texSize.y);\
        realCenter.y = center.y / texSize.y;\
        float maxX = realCenter.x + circleRadius;\
        float minX = realCenter.x - circleRadius;\
        float maxY = realCenter.y + circleRadius;\
        float minY = realCenter.y - circleRadius;\
        if (uv.x > minX && uv.x < maxX && uv.y > minY && uv.y < maxY) {\
            float relX = uv.x - realCenter.x;\
            float relY = uv.y - realCenter.y;\
            float ang =  atan(relY, relX);\
            float dist = sqrt(relX * relX + relY * relY);\
            if (dist <= circleRadius) {\
                float newRad = dist * ((maxZoom * dist / circleRadius) + minZoom);\
                float newX = realCenter.x + cos(ang) * newRad;\
                newX *= (texSize.y / texSize.x);\
                float newY = realCenter.y + sin(ang) * newRad;\
                gl_FragColor = YuvToRgb(vec2(newX, newY));\
            } else {\
                gl_FragColor = YuvToRgb(v_texcoord);\
            }\
        } else {\
            gl_FragColor = YuvToRgb(v_texcoord);\
        }\
    }";

// Fish Eye Filter
static const char kFragmentShader7[] =
    "#version 100\n\
    precision highp float;\
    const float PI = 3.1415926535;\
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    void main() {\
        float aperture = 158.0;\
        float apertureHalf = 0.5 * aperture * (PI / 180.0);\
        float maxFactor = sin(apertureHalf);\
        vec2 uv;\
        vec2 xy = 2.0 * v_texcoord.xy - 1.0;\
        float d = length(xy);\
        if (d < (2.0 - maxFactor)) {\
            d = length(xy * maxFactor);\
            float z = sqrt(1.0 - d * d);\
            float r = atan(d, z) / PI;\
            float phi = atan(xy.y, xy.x);\
            uv.x = r * cos(phi) + 0.5;\
            uv.y = r * sin(phi) + 0.5;\
        } else {\
            uv = v_texcoord.xy;\
        }\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, uv).r;\
        u = texture2D(s_textureU, uv).r;\
        v = texture2D(s_textureV, uv).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        gl_FragColor = vec4(r, g, b, 1.0);\
    }";

// Predator Thermal Vision Filter
static const char kFragmentShader8[] =
    "#version 100\n\
    precision highp float;\
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    void main() {\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, v_texcoord).r;\
        u = texture2D(s_textureU, v_texcoord).r;\
        v = texture2D(s_textureV, v_texcoord).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        vec3 color = vec3(r, g, b);\
        vec2 uv = v_texcoord.xy;\
        vec3 colors[3];\
        colors[0] = vec3(0.,0.,1.);\
        colors[1] = vec3(1.,1.,0.);\
        colors[2] = vec3(1.,0.,0.);\
        float lum = (color.r + color.g + color.b)/3.;\
        int idx = (lum < 0.5) ? 0 : 1;\
        vec3 rgb = mix(colors[idx],colors[idx+1],(lum-float(idx)*0.5)/0.5);\
        gl_FragColor = vec4(rgb, 1.0);\
    }";

// Pixelation Filter
static const char kFragmentShader9[] =
    "#version 100\n\
    precision highp float;\
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    void main() {\
        float w = 640.;\
        float h = 480.;\
        vec2 pixelSize = vec2(5.,5.);\
        vec2 uv = v_texcoord.xy;\
        float dx = pixelSize.x*(1./w);\
        float dy = pixelSize.y*(1./h);\
        vec2 coord = vec2(dx*floor(uv.x/dx),\
        dy*floor(uv.y/dy));\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, coord).r;\
        u = texture2D(s_textureU, coord).r;\
        v = texture2D(s_textureV, coord).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        gl_FragColor = vec4(r, g, b, 1.0);\
    }";

// Cross Stitching Filter
static const char kFragmentShader10[] =
    "#version 100\n\
    precision highp float;\
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    vec4 YuvToRgb(vec2 uv) {\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, uv).r;\
        u = texture2D(s_textureU, uv).r;\
        v = texture2D(s_textureV, uv).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        return vec4(r, g, b, 1.0);\
    }\
    vec4 CrossStitching(vec2 uv) {\
        float w = 640.;\
        float h = 480.;\
        float stitching_size = 5.0;\
        int invert = 0;\
        vec4 color = vec4(0.0);\
        float size = stitching_size;\
        vec2 cPos = uv * vec2(w, h);\
        vec2 tlPos = floor(cPos / vec2(size, size));\
        tlPos *= size;\
        int remX = int(mod(cPos.x, size));\
        int remY = int(mod(cPos.y, size));\
        if (remX == 0 && remY == 0)\
            tlPos = cPos;\
        vec2 blPos = tlPos;\
        blPos.y += (size - 1.0);\
        if ((remX == remY) || (((int(cPos.x) - int(blPos.x)) == (int(blPos.y) - int(cPos.y))))) {\
            if (invert == 1)\
                color = vec4(0.2, 0.15, 0.05, 1.0);\
            else\
                color = YuvToRgb(tlPos * vec2(1.0/w, 1.0/h)) * 1.4;\
        } else {\
            if (invert == 1)\
                color = YuvToRgb(tlPos * vec2(1.0/w, 1.0/h)) * 1.4;\
            else\
                color = vec4(0.0, 0.0, 0.0, 1.0);\
        }\
        return color;\
    }\
    void main() {\
        gl_FragColor = CrossStitching(v_texcoord);\
    }";

// Emboss Filter
static const char kFragmentShader11[] =
    "#version 100\n \
    precision highp float; \
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    vec4 YuvToRgb(vec2 uv) {\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, uv).r;\
        u = texture2D(s_textureU, uv).r;\
        v = texture2D(s_textureV, uv).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        return vec4(r, g, b, 1.0);\
    }\
    void main() {\
        vec4 color;\
        color.rgb = vec3(0.5);\
        vec2 onePixel = vec2(1.0 / 480.0, 1.0 / 320.0);\
        color -= YuvToRgb(v_texcoord - onePixel) * 5.0;\
        color += YuvToRgb(v_texcoord + onePixel) * 5.0;\
        color.rgb = vec3((color.r + color.g + color.b) / 3.0);\
        gl_FragColor = vec4(color.rgb, 1.0);\
    }";

// Edge Detection Filter
static const char kFragmentShader12[] =
    "#version 100\n \
    precision highp float; \
    varying vec2 v_texcoord;\
    uniform lowp sampler2D s_textureY;\
    uniform lowp sampler2D s_textureU;\
    uniform lowp sampler2D s_textureV;\
    vec4 YuvToRgb(vec2 uv) {\
        float y, u, v, r, g, b;\
        y = texture2D(s_textureY, uv).r;\
        u = texture2D(s_textureU, uv).r;\
        v = texture2D(s_textureV, uv).r;\
        u = u - 0.5;\
        v = v - 0.5;\
        r = y + 1.403 * v;\
        g = y - 0.344 * u - 0.714 * v;\
        b = y + 1.770 * u;\
        return vec4(r, g, b, 1.0);\
    }\
    void main() {\
        vec2 texSize = vec2(640, 480);\
        vec2 pos = vec2(v_texcoord.x, 1.0 - v_texcoord.y);\
        vec2 onePixel = vec2(1, 1) / texSize;\
        vec4 color = vec4(0);\
        mat3 edgeDetectionKernel = mat3(\
            -1, -1, -1,\
            -1, 8, -1,\
            -1, -1, -1\
        );\
        for(int i = 0; i < 3; i++) {\
            for(int j = 0; j < 3; j++) {\
                vec2 samplePos = pos + vec2(i - 1 , j - 1) * onePixel;\
                vec4 sampleColor = YuvToRgb(samplePos);\
                sampleColor *= edgeDetectionKernel[i][j];\
                color += sampleColor;\
            }\
        }\
        color.a = 1.0;\
        gl_FragColor = color;\
    }";

#endif //_H_GL_SHADER_H_
