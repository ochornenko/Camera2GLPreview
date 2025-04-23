#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 uv;
layout (binding = 0) uniform UniformBufferObject
{
    mat4 rotation;
    mat4 scale;
} ubo;
layout (location = 0) out vec2 texcoord;

void main() {
    vec4 transformed = ubo.rotation * ubo.scale * vec4(uv - vec2(0.5), 0.0, 1.0);
    texcoord = transformed.xy + vec2(0.5);
    gl_Position = pos;
}
