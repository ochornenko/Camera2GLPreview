#ifndef _VK_UTILS_H_
#define _VK_UTILS_H_

#include <vulkan_wrapper.h>

VkResult buildShader(const char *data, VkShaderStageFlagBits type, VkDevice vkDevice, VkShaderModule* shaderOut);

#endif //_VK_UTILS_H_
