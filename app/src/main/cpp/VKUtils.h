#ifndef _VK_UTILS_H_
#define _VK_UTILS_H_

#include <android/asset_manager_jni.h>
#include <vulkan/vulkan.h>

bool createShaderModuleFromAsset(VkDevice device, const char *shaderFilePath,
                                 AAssetManager *assetManager, VkShaderModule *shaderModule);

#endif //_VK_UTILS_H_
