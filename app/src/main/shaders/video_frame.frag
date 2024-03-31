#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D tex[3];
layout (location = 0) in vec2 texcoord;
layout (location = 0) out vec4 uFragColor;

void main() {
    float y, u, v, r, g, b;
    y = texture(tex[0], texcoord).r;
    u = texture(tex[1], texcoord).r;
    v = texture(tex[2], texcoord).r;
    u = u - 0.5;
    v = v - 0.5;
    r = y + 1.403 * v;
    g = y - 0.344 * u - 0.714 * v;
    b = y + 1.770 * u;
    uFragColor = vec4(r, g, b, 1.0);
}
