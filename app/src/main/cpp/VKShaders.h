#ifndef _VK_SHADER_H_
#define _VK_SHADER_H_

static const char kVertexShader[] = \
    "#version 400\n \
    #extension GL_ARB_separate_shader_objects : enable\n \
    #extension GL_ARB_shading_language_420pack : enable\n \
    layout (location = 0) in vec4 pos; \
    layout (location = 1) in vec2 uv; \
    layout (binding = 0) uniform UniformBufferObject \
    { \
        mat4 projection; \
        mat4 rotation; \
        mat4 scale; \
    } ubo; \
    layout (location = 0) out vec2 texcoord; \
    void main() { \
        texcoord = uv; \
        gl_Position = ubo.projection * ubo.rotation * ubo.scale * vec4(pos.xyz, 1.0); \
    }";

static const char kFragmentShader[] = \
    "#version 400\n \
    #extension GL_ARB_separate_shader_objects : enable\n \
    #extension GL_ARB_shading_language_420pack : enable\n \
    layout (binding = 1) uniform sampler2D tex[3]; \
    layout (location = 0) in vec2 texcoord; \
    layout (location = 0) out vec4 uFragColor; \
    void main() { \
        float y, u, v, r, g, b; \
        y = texture(tex[0], texcoord).r; \
        u = texture(tex[1], texcoord).r; \
        v = texture(tex[2], texcoord).r; \
        u = u - 0.5; \
        v = v - 0.5; \
        r = y + 1.403 * v; \
        g = y - 0.344 * u - 0.714 * v; \
        b = y + 1.770 * u; \
        uFragColor = vec4(r, g, b, 1.0); \
    }";

#endif //_VK_SHADER_H_
