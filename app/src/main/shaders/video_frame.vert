#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 uv;
layout (binding = 0) uniform UniformBufferObject
{
    mat4 projection;
    mat4 rotation;
    mat4 scale;
} ubo;
layout (location = 0) out vec2 texcoord;

void main() {
    texcoord = uv;
    gl_Position = ubo.projection * ubo.rotation * ubo.scale * vec4(pos.xyz, 1.0);
}
