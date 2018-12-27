#include "VKUtils.h"
#include "Log.h"

#include <shaderc/shaderc.hpp>

shaderc_shader_kind getShadercShaderType(VkShaderStageFlagBits type)
{
    switch (type)
    {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return shaderc_glsl_vertex_shader;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return shaderc_glsl_fragment_shader;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return shaderc_glsl_tess_control_shader;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return shaderc_glsl_tess_evaluation_shader;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return shaderc_glsl_geometry_shader;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return shaderc_glsl_compute_shader;
        default:
            LOGE("invalid VKShaderStageFlagBits", "type = %08x", type);
    }
    return static_cast<shaderc_shader_kind>(-1);
}

// Create VK shader module from given glsl shader source text
VkResult buildShader(const char *data, VkShaderStageFlagBits type, VkDevice vkDevice,
                             VkShaderModule* shaderOut) {

    // compile into spir-V shader
    shaderc_compiler_t compiler = shaderc_compiler_initialize();
    shaderc_compilation_result_t spvShader = shaderc_compile_into_spv(compiler, data,
            strlen(data), getShadercShaderType(type), "shaderc_error", "main", nullptr);
    int status = shaderc_result_get_compilation_status(spvShader);
    if (status != shaderc_compilation_status_success) {
        LOGE("compilation status", "error = %d", status);
        return static_cast<VkResult>(-1);
    }

    // build vulkan shader module
    VkShaderModuleCreateInfo shaderModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .codeSize = shaderc_result_get_length(spvShader),
            .pCode = (const uint32_t*)shaderc_result_get_bytes(spvShader),
            .flags = 0,
    };
    VkResult result = vkCreateShaderModule(vkDevice, &shaderModuleCreateInfo, nullptr, shaderOut);

    shaderc_result_release(spvShader);
    shaderc_compiler_release(compiler);

    return result;
}
