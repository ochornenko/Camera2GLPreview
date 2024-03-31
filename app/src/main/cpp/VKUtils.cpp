#include "VKUtils.h"
#include "Log.h"

#include <cassert>
#include <vector>

bool createShaderModuleFromAsset(VkDevice device, const char *shaderFilePath,
                                 AAssetManager *assetManager,
                                 VkShaderModule *shaderModule) {
    // Read shader file from asset.
    AAsset *shaderFile = AAssetManager_open(assetManager, shaderFilePath,
                                            AASSET_MODE_BUFFER);
    RET_CHECK(shaderFile != nullptr);
    const size_t shaderSize = AAsset_getLength(shaderFile);
    std::vector<char> shader(shaderSize);
    int status = AAsset_read(shaderFile, shader.data(), shaderSize);
    AAsset_close(shaderFile);
    RET_CHECK(status >= 0);

    // Create shader module.
    const VkShaderModuleCreateInfo shaderDesc = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .flags = 0,
            .codeSize = shaderSize,
            .pCode = reinterpret_cast<const uint32_t *>(shader.data()),
    };
    CALL_VK_RET(vkCreateShaderModule(device, &shaderDesc, nullptr, shaderModule))
    return true;
}
