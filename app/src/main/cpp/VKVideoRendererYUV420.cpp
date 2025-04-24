#include "VKVideoRendererYUV420.h"
#include "VKUtils.h"
#include "CommonUtils.h"
#include "Log.h"

#include <cassert>
#include <vector>
#include <cstring>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

VKVideoRendererYUV420::VKVideoRendererYUV420()
        : texType{tTexY, tTexU, tTexV},
          m_pBuffer(nullptr),
          m_indexCount(0) {
    m_deviceInfo.initialized = false;
}

VKVideoRendererYUV420::~VKVideoRendererYUV420() {
    deleteCommandPool();
    deleteGraphicsPipeline();
    deleteTextures();
    deleteUniformBuffers();
    deleteBuffers();
    deleteRenderPass();
    deleteSwapChain();

    vkDestroyDevice(m_deviceInfo.device, nullptr);
    vkDestroyInstance(m_deviceInfo.instance, nullptr);

    m_deviceInfo.initialized = false;
}

void VKVideoRendererYUV420::createRenderPipeline() {
    createRenderPass();
    createFrameBuffers(); // Create 2 frame buffers.
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createTextures();
    createProgram(nullptr, nullptr); // Create graphics pipeline
    createDescriptorSet();
    createCommandPool();

    m_deviceInfo.initialized = true;
}

void VKVideoRendererYUV420::init(ANativeWindow *window, AAssetManager *assetManager, size_t width,
                                 size_t height) {
    m_surfaceWidth = width;
    m_surfaceHeight = height;

    m_assetManager = assetManager;

    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "camera2GLPreview",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "camera",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_MAKE_VERSION(1, 0, 0),
    };

    createDevice(window, &appInfo);
    createSwapChain();
}

void VKVideoRendererYUV420::render() {
    uint32_t nextIndex;
    // Get the framebuffer index we should draw in
    CALL_VK(vkAcquireNextImageKHR(m_deviceInfo.device, m_swapchainInfo.swapchain, UINT64_MAX,
                                  m_render.semaphore, VK_NULL_HANDLE, &nextIndex))
    CALL_VK(vkResetFences(m_deviceInfo.device, 1, &m_render.fence))

    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_render.semaphore,
            .pWaitDstStageMask = &waitStageMask,
            .commandBufferCount = 1,
            .pCommandBuffers = &m_render.cmdBuffer[nextIndex],
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr
    };
    CALL_VK(vkQueueSubmit(m_deviceInfo.queue, 1, &submitInfo, m_render.fence))
    CALL_VK(vkWaitForFences(m_deviceInfo.device, 1, &m_render.fence, VK_TRUE, 100000000))

    VkResult result;
    VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .swapchainCount = 1,
            .pSwapchains = &m_swapchainInfo.swapchain,
            .pImageIndices = &nextIndex,
            .pResults = &result,
    };
    vkQueuePresentKHR(m_deviceInfo.queue, &presentInfo);
}

void VKVideoRendererYUV420::draw(uint8_t *buffer, size_t length, size_t width, size_t height,
                                 float rotation, bool mirror) {
    m_pBuffer = buffer;
    m_rotation = rotation;
    m_mirror = mirror;

    if (isInitialized() && (m_frameWidth != width || m_frameHeight != height)) {
        m_frameWidth = width;
        m_frameHeight = height;

        deleteUniformBuffers();
        deleteTextures();
        deleteCommandPool();

        createUniformBuffers();
        createTextures();
        updateDescriptorSet();
        createCommandPool();
    } else {
        m_frameWidth = width;
        m_frameHeight = height;
    }

    if (!isInitialized()) {
        createRenderPipeline();
    } else {
        updateTextures();
    }

    if (isInitialized()) {
        render();
    }
}

void VKVideoRendererYUV420::setParameters(uint32_t params) {
    m_params = params;
}

uint32_t VKVideoRendererYUV420::getParameters() {
    return m_params;
}

bool VKVideoRendererYUV420::createTextures() {
    for (int i = 0; i < kTextureCount; i++) {
        loadTexture(m_pBuffer, texType[i], m_frameWidth, m_frameHeight, &textures[i],
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        const VkSamplerCreateInfo sampler{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .pNext = nullptr,
                .magFilter = VK_FILTER_NEAREST,
                .minFilter = VK_FILTER_NEAREST,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipLodBias = 0.0f,
                .maxAnisotropy = 1,
                .compareOp = VK_COMPARE_OP_NEVER,
                .minLod = 0.0f,
                .maxLod = 0.0f,
                .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                .unnormalizedCoordinates = VK_FALSE,
        };
        VkImageViewCreateInfo view{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = VK_NULL_HANDLE,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = kTextureFormat,
                .components = {
                        VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                        VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };

        CALL_VK(vkCreateSampler(m_deviceInfo.device, &sampler, nullptr, &textures[i].sampler))
        view.image = textures[i].image;
        CALL_VK(vkCreateImageView(m_deviceInfo.device, &view, nullptr, &textures[i].view))
    }

    return true;
}

bool VKVideoRendererYUV420::updateTextures() {
    for (int i = 0; i < kTextureCount; i++) {
        size_t offset = getBufferOffset(&textures[i], texType[i], m_frameWidth, m_frameHeight);
        copyTextureData(&textures[i], m_pBuffer + offset);
    }
    return true;
}

void VKVideoRendererYUV420::deleteTextures() const {
    for (auto &texture: textures) {
        vkDestroyImageView(m_deviceInfo.device, texture.view, nullptr);
        vkDestroyImage(m_deviceInfo.device, texture.image, nullptr);
        vkDestroySampler(m_deviceInfo.device, texture.sampler, nullptr);
        vkUnmapMemory(m_deviceInfo.device, texture.mem);
        vkFreeMemory(m_deviceInfo.device, texture.mem, nullptr);
    }
}

void VKVideoRendererYUV420::deleteRenderPass() const {
    vkDestroyRenderPass(m_deviceInfo.device, m_render.renderPass, nullptr);
}

int VKVideoRendererYUV420::createProgram(const char *pVertexSource, const char *pFragmentSource) {
    return createGraphicsPipeline();
}

void
VKVideoRendererYUV420::createDevice(ANativeWindow *platformWindow, VkApplicationInfo *appInfo) {
    std::vector<const char *> instance_extensions;
    std::vector<const char *> device_extensions;

    instance_extensions.push_back("VK_KHR_surface");
    instance_extensions.push_back("VK_KHR_android_surface");

    device_extensions.push_back("VK_KHR_swapchain");

    // Create the Vulkan instance
    VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = appInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size()),
            .ppEnabledExtensionNames = instance_extensions.data(),
    };
    CALL_VK(vkCreateInstance(&instanceCreateInfo, nullptr, &m_deviceInfo.instance))
    VkAndroidSurfaceCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = platformWindow
    };

    CALL_VK(vkCreateAndroidSurfaceKHR(m_deviceInfo.instance, &createInfo, nullptr,
                                      &m_deviceInfo.surface))
    // Find one GPU to use:
    // On Android, every GPU device is equal -- supporting
    // graphics/compute/present
    // for this sample, we use the very first GPU device found on the system
    uint32_t gpuCount = 0;
    CALL_VK(vkEnumeratePhysicalDevices(m_deviceInfo.instance, &gpuCount, nullptr))
    VkPhysicalDevice tmpGpus[gpuCount];
    CALL_VK(vkEnumeratePhysicalDevices(m_deviceInfo.instance, &gpuCount, tmpGpus))
    m_deviceInfo.physicalDevice = tmpGpus[0];  // Pick up the first GPU Device

    vkGetPhysicalDeviceMemoryProperties(m_deviceInfo.physicalDevice,
                                        &m_deviceInfo.memoryProperties);

    // Find a GFX queue family
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(m_deviceInfo.physicalDevice, &queueFamilyCount,
                                             nullptr);
    assert(queueFamilyCount);
    std::vector <VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_deviceInfo.physicalDevice, &queueFamilyCount,
                                             queueFamilyProperties.data());

    uint32_t queueFamilyIndex;
    for (queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount;
         queueFamilyIndex++) {
        if (queueFamilyProperties[queueFamilyIndex].queueFlags &
            VK_QUEUE_GRAPHICS_BIT) {
            break;
        }
    }
    assert(queueFamilyIndex < queueFamilyCount);
    m_deviceInfo.queueFamilyIndex = queueFamilyIndex;
    // Create a logical device (vulkan device)
    float priorities[] = {
            1.0f,
    };
    VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = m_deviceInfo.queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = priorities,
    };

    VkDeviceCreateInfo deviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data(),
            .pEnabledFeatures = nullptr,
    };

    CALL_VK(vkCreateDevice(m_deviceInfo.physicalDevice, &deviceCreateInfo, nullptr,
                           &m_deviceInfo.device))
    vkGetDeviceQueue(m_deviceInfo.device, 0, 0, &m_deviceInfo.queue);
}

void VKVideoRendererYUV420::createSwapChain() {
    memset(&m_swapchainInfo, 0, sizeof(m_swapchainInfo));

    // Get the surface capabilities because:
    //   - It contains the minimal and max length of the chain, we will need it
    //   - It's necessary to query the supported surface format (R8G8B8A8 for instance ...)
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_deviceInfo.physicalDevice, m_deviceInfo.surface,
                                              &surfaceCapabilities);
    // Query the list of supported surface format and choose one we like
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_deviceInfo.physicalDevice, m_deviceInfo.surface,
                                         &formatCount, nullptr);

    std::unique_ptr < VkSurfaceFormatKHR[] > formats = std::make_unique<VkSurfaceFormatKHR[]>(
            formatCount);

    vkGetPhysicalDeviceSurfaceFormatsKHR(m_deviceInfo.physicalDevice, m_deviceInfo.surface,
                                         &formatCount, formats.get());
    uint32_t chosenFormat;
    for (chosenFormat = 0; chosenFormat < formatCount; chosenFormat++) {
        if (formats[chosenFormat].format == VK_FORMAT_R8G8B8A8_UNORM) break;
    }
    assert(chosenFormat < formatCount);

    m_swapchainInfo.displaySize = surfaceCapabilities.currentExtent;
    m_swapchainInfo.displayFormat = formats[chosenFormat].format;

    // Create a swap chain (here we choose the minimum available number of surface in the chain)
    VkSwapchainCreateInfoKHR swapchainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .surface = m_deviceInfo.surface,
            .minImageCount = surfaceCapabilities.minImageCount,
            .imageFormat = formats[chosenFormat].format,
            .imageColorSpace = formats[chosenFormat].colorSpace,
            .imageExtent = surfaceCapabilities.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &m_deviceInfo.queueFamilyIndex,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = VK_FALSE,
            .oldSwapchain = VK_NULL_HANDLE,
    };
    CALL_VK(vkCreateSwapchainKHR(m_deviceInfo.device, &swapchainCreateInfo, nullptr,
                                 &m_swapchainInfo.swapchain))

    // Get the length of the created swap chain
    CALL_VK(vkGetSwapchainImagesKHR(m_deviceInfo.device, m_swapchainInfo.swapchain,
                                    &m_swapchainInfo.swapchainLength, nullptr))
}

void VKVideoRendererYUV420::deleteSwapChain() const {
    for (int i = 0; i < m_swapchainInfo.swapchainLength; i++) {
        vkDestroyFramebuffer(m_deviceInfo.device, m_swapchainInfo.framebuffers[i], nullptr);
        vkDestroyImageView(m_deviceInfo.device, m_swapchainInfo.displayViews[i], nullptr);
    }

    vkDestroySwapchainKHR(m_deviceInfo.device, m_swapchainInfo.swapchain, nullptr);
}

void VKVideoRendererYUV420::deleteCommandPool() const {
    vkFreeCommandBuffers(m_deviceInfo.device, m_render.cmdPool, m_render.cmdBufferLen,
                         m_render.cmdBuffer.get());
    vkDestroyCommandPool(m_deviceInfo.device, m_render.cmdPool, nullptr);
    vkDestroyFence(m_deviceInfo.device, m_render.fence, nullptr);
    vkDestroySemaphore(m_deviceInfo.device, m_render.semaphore, nullptr);
}

void VKVideoRendererYUV420::deleteGraphicsPipeline() {
    if (m_gfxPipeline.pipeline == VK_NULL_HANDLE) return;
    vkDestroyPipeline(m_deviceInfo.device, m_gfxPipeline.pipeline, nullptr);
    vkDestroyPipelineCache(m_deviceInfo.device, m_gfxPipeline.cache, nullptr);
    vkFreeDescriptorSets(m_deviceInfo.device, m_gfxPipeline.descPool, 1, &m_gfxPipeline.descSet);
    vkDestroyDescriptorPool(m_deviceInfo.device, m_gfxPipeline.descPool, nullptr);
    vkDestroyPipelineLayout(m_deviceInfo.device, m_gfxPipeline.layout, nullptr);
}

void VKVideoRendererYUV420::createFrameBuffers(VkImageView depthView) {
    // query display attachment to swapchain
    uint32_t swapchainImagesCount = 0;
    CALL_VK(vkGetSwapchainImagesKHR(m_deviceInfo.device, m_swapchainInfo.swapchain,
                                    &swapchainImagesCount, nullptr))
    m_swapchainInfo.displayImages = std::make_unique<VkImage[]>(swapchainImagesCount);
    CALL_VK(vkGetSwapchainImagesKHR(m_deviceInfo.device, m_swapchainInfo.swapchain,
                                    &swapchainImagesCount,
                                    m_swapchainInfo.displayImages.get()))

    // create image view for each swapchain image
    m_swapchainInfo.displayViews = std::make_unique<VkImageView[]>(swapchainImagesCount);
    for (uint32_t i = 0; i < swapchainImagesCount; i++) {
        VkImageViewCreateInfo viewCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = m_swapchainInfo.displayImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_swapchainInfo.displayFormat,
                .components = {
                        .r = VK_COMPONENT_SWIZZLE_R,
                        .g = VK_COMPONENT_SWIZZLE_G,
                        .b = VK_COMPONENT_SWIZZLE_B,
                        .a = VK_COMPONENT_SWIZZLE_A,
                },
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                },
        };
        CALL_VK(vkCreateImageView(m_deviceInfo.device, &viewCreateInfo, nullptr,
                                  &m_swapchainInfo.displayViews[i]))
    }

    // create a framebuffer from each swapchain image
    m_swapchainInfo.framebuffers = std::make_unique<VkFramebuffer[]>(
            m_swapchainInfo.swapchainLength);
    for (uint32_t i = 0; i < m_swapchainInfo.swapchainLength; i++) {
        VkImageView attachments[2] = {
                m_swapchainInfo.displayViews[i],
                depthView,
        };
        VkFramebufferCreateInfo fbCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .renderPass = m_render.renderPass,
                .attachmentCount = 1,  // 2 if using depth
                .pAttachments = attachments,
                .width = static_cast<uint32_t>(m_swapchainInfo.displaySize.width),
                .height = static_cast<uint32_t>(m_swapchainInfo.displaySize.height),
                .layers = 1,
        };
        fbCreateInfo.attachmentCount = (depthView == VK_NULL_HANDLE ? 1 : 2);

        CALL_VK(vkCreateFramebuffer(m_deviceInfo.device, &fbCreateInfo, nullptr,
                                    &m_swapchainInfo.framebuffers[i]))
    }
}

// Helper function to transition color buffer layout
void VKVideoRendererYUV420::setImageLayout(VkCommandBuffer cmdBuffer, VkImage image,
                                           VkImageLayout oldImageLayout,
                                           VkImageLayout newImageLayout,
                                           VkPipelineStageFlags srcStages,
                                           VkPipelineStageFlags destStages) {
    VkImageMemoryBarrier imageMemoryBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = oldImageLayout,
            .newLayout = newImageLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            },
    };

    switch (oldImageLayout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        default:
            break;
    }

    switch (newImageLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        default:
            break;
    }

    vkCmdPipelineBarrier(cmdBuffer, srcStages, destStages, 0, 0, nullptr, 0, nullptr, 1,
                         &imageMemoryBarrier);
}

// Create Graphics Pipeline
VkResult VKVideoRendererYUV420::createGraphicsPipeline() {
    memset(&m_gfxPipeline, 0, sizeof(m_gfxPipeline));

    const VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[2]{
            {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = nullptr
            },
            {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = kTextureCount,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr
            }};
    const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = 2,
            .pBindings = descriptorSetLayoutBinding,
    };
    CALL_VK(vkCreateDescriptorSetLayout(m_deviceInfo.device,
                                        &descriptorSetLayoutCreateInfo, nullptr,
                                        &m_gfxPipeline.descLayout))
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .setLayoutCount = 1,
            .pSetLayouts = &m_gfxPipeline.descLayout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
    };
    CALL_VK(vkCreatePipelineLayout(m_deviceInfo.device, &pipelineLayoutCreateInfo,
                                   nullptr, &m_gfxPipeline.layout))

    // No dynamic state in that tutorial
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .dynamicStateCount = 0,
            .pDynamicStates = nullptr};

    VkShaderModule vertexShader, fragmentShader;

    RET_CHECK(createShaderModuleFromAsset(m_deviceInfo.device, "shaders/video_frame.vert.spv",
                                          m_assetManager, &vertexShader));
    RET_CHECK(createShaderModuleFromAsset(m_deviceInfo.device, "shaders/video_frame.frag.spv",
                                          m_assetManager, &fragmentShader));

    // Specify vertex and fragment shader stages
    VkPipelineShaderStageCreateInfo shaderStages[2]{
            {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = vertexShader,
                    .pName = "main",
                    .pSpecializationInfo = nullptr,
            },
            {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = fragmentShader,
                    .pName = "main",
                    .pSpecializationInfo = nullptr,
            }
    };

    VkViewport viewports{
            .x = 0,
            .y = 0,
            .width = (float) m_swapchainInfo.displaySize.width,
            .height = (float) m_swapchainInfo.displaySize.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
            .offset = {.x = 0, .y = 0},
            .extent = m_swapchainInfo.displaySize
    };
    // Specify viewport info
    VkPipelineViewportStateCreateInfo viewportInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .viewportCount = 1,
            .pViewports = &viewports,
            .scissorCount = 1,
            .pScissors = &scissor,
    };

    // Specify multisample info
    VkSampleMask sampleMask = ~0u;
    VkPipelineMultisampleStateCreateInfo multisampleInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 0,
            .pSampleMask = &sampleMask,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
    };

    // Specify color blend state
    VkPipelineColorBlendAttachmentState attachmentStates{
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &attachmentStates,
    };

    // Specify rasterizer info
    VkPipelineRasterizationStateCreateInfo rasterInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1,
    };

    // Specify input assembler state
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
    };

    // Specify vertex input state
    VkVertexInputBindingDescription vertex_input_bindings{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    VkVertexInputAttributeDescription vertex_input_attributes[2]{
            {
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = (uint32_t) offsetof(Vertex, pos),
            },
            {
                    .location = 1,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = (uint32_t) offsetof(Vertex, uv),
            }
    };
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &vertex_input_bindings,
            .vertexAttributeDescriptionCount = 2,
            .pVertexAttributeDescriptions = vertex_input_attributes,
    };

    // Create the pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,  // reserved, must be 0
            .initialDataSize = 0,
            .pInitialData = nullptr,
    };

    CALL_VK(vkCreatePipelineCache(m_deviceInfo.device, &pipelineCacheInfo, nullptr,
                                  &m_gfxPipeline.cache))

    // Create the pipeline
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pTessellationState = nullptr,
            .pViewportState = &viewportInfo,
            .pRasterizationState = &rasterInfo,
            .pMultisampleState = &multisampleInfo,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &colorBlendInfo,
            .pDynamicState = &dynamicStateInfo,
            .layout = m_gfxPipeline.layout,
            .renderPass = m_render.renderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
    };

    VkResult pipelineResult = vkCreateGraphicsPipelines(
            m_deviceInfo.device, m_gfxPipeline.cache, 1, &pipelineCreateInfo, nullptr,
            &m_gfxPipeline.pipeline);

    // We don't need the shaders anymore, we can release their memory
    vkDestroyShaderModule(m_deviceInfo.device, vertexShader, nullptr);
    vkDestroyShaderModule(m_deviceInfo.device, fragmentShader, nullptr);

    return pipelineResult;
}

void VKVideoRendererYUV420::updateDescriptorSet() {
    VkDescriptorBufferInfo bufferInfo{
            bufferInfo.buffer = m_buffers.uboBuffer,
            bufferInfo.offset = 0,
            bufferInfo.range = sizeof(UniformBufferObject)
    };

    VkDescriptorImageInfo texDsts[kTextureCount];
    memset(texDsts, 0, sizeof(texDsts));
    for (int32_t idx = 0; idx < kTextureCount; idx++) {
        texDsts[idx].sampler = textures[idx].sampler;
        texDsts[idx].imageView = textures[idx].view;
        texDsts[idx].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    VkWriteDescriptorSet writeDst[2]{
            {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = m_gfxPipeline.descSet,
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pImageInfo = nullptr,
                    .pBufferInfo = &bufferInfo,
                    .pTexelBufferView = nullptr
            },
            {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = m_gfxPipeline.descSet,
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = kTextureCount,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .pImageInfo = texDsts,
                    .pBufferInfo = nullptr,
                    .pTexelBufferView = nullptr
            }
    };
    vkUpdateDescriptorSets(m_deviceInfo.device, 2, writeDst, 0, nullptr);
}

// initialize descriptor set
void VKVideoRendererYUV420::createDescriptorSet() {
    const VkDescriptorPoolSize poolSizes[2]{
            {
                    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1
            },
            {
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = kTextureCount
            }
    };
    const VkDescriptorPoolCreateInfo descriptor_pool = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .maxSets = 2,
            .poolSizeCount = 2,
            .pPoolSizes = poolSizes,
    };

    CALL_VK(vkCreateDescriptorPool(m_deviceInfo.device, &descriptor_pool, nullptr,
                                   &m_gfxPipeline.descPool))

    VkDescriptorSetAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_gfxPipeline.descPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_gfxPipeline.descLayout};
    CALL_VK(vkAllocateDescriptorSets(m_deviceInfo.device, &alloc_info, &m_gfxPipeline.descSet))

    updateDescriptorSet();
}

void VKVideoRendererYUV420::createCommandPool() {
// Create a pool of command buffers to allocate command buffer from
    VkCommandPoolCreateInfo cmdPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = 0,
    };
    CALL_VK(vkCreateCommandPool(m_deviceInfo.device, &cmdPoolCreateInfo, nullptr,
                                &m_render.cmdPool))

    // Record a command buffer that just clear the screen
    // 1 command buffer draw in 1 framebuffer
    // In our case we need 2 command as we have 2 framebuffer
    m_render.cmdBufferLen = m_swapchainInfo.swapchainLength;
    m_render.cmdBuffer = std::make_unique<VkCommandBuffer[]>(m_swapchainInfo.swapchainLength);
    VkCommandBufferAllocateInfo cmdBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_render.cmdPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = m_render.cmdBufferLen,
    };
    CALL_VK(vkAllocateCommandBuffers(m_deviceInfo.device, &cmdBufferCreateInfo,
                                     m_render.cmdBuffer.get()))

    for (int bufferIndex = 0; bufferIndex < m_swapchainInfo.swapchainLength; bufferIndex++) {
        // We start by creating and declare the "beginning" our command buffer
        VkCommandBufferBeginInfo cmdBufferBeginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = 0,
                .pInheritanceInfo = nullptr,
        };
        CALL_VK(vkBeginCommandBuffer(m_render.cmdBuffer[bufferIndex], &cmdBufferBeginInfo))

        // transition the buffer into color attachment
        setImageLayout(m_render.cmdBuffer[bufferIndex],
                       m_swapchainInfo.displayImages[bufferIndex],
                       VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        // Now we start a render pass. Any draw command has to be recorded in a render pass
        VkClearValue clearValues;
        clearValues.color.float32[0] = 0.0f;
        clearValues.color.float32[1] = 0.0f;
        clearValues.color.float32[2] = 0.0f;
        clearValues.color.float32[3] = 1.0f;

        VkRenderPassBeginInfo renderPassBeginInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = m_render.renderPass,
                .framebuffer = m_swapchainInfo.framebuffers[bufferIndex],
                .renderArea = {.offset = {.x = 0, .y = 0},
                        .extent = m_swapchainInfo.displaySize},
                .clearValueCount = 1,
                .pClearValues = &clearValues};
        vkCmdBeginRenderPass(m_render.cmdBuffer[bufferIndex], &renderPassBeginInfo,
                             VK_SUBPASS_CONTENTS_INLINE);
        // Bind what is necessary to the command buffer
        vkCmdBindPipeline(m_render.cmdBuffer[bufferIndex],
                          VK_PIPELINE_BIND_POINT_GRAPHICS, m_gfxPipeline.pipeline);
        vkCmdBindDescriptorSets(m_render.cmdBuffer[bufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_gfxPipeline.layout, 0, 1, &m_gfxPipeline.descSet, 0, nullptr);
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(m_render.cmdBuffer[bufferIndex], 0, 1, &m_buffers.vertexBuffer,
                               &offset);

        vkCmdBindIndexBuffer(m_render.cmdBuffer[bufferIndex], m_buffers.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(m_render.cmdBuffer[bufferIndex], m_indexCount, 1, 0, 0, 0);

        vkCmdEndRenderPass(m_render.cmdBuffer[bufferIndex]);
        setImageLayout(m_render.cmdBuffer[bufferIndex],
                       m_swapchainInfo.displayImages[bufferIndex],
                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                       VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        CALL_VK(vkEndCommandBuffer(m_render.cmdBuffer[bufferIndex]))
    }

    // We need to create a fence to be able, in the main loop, to wait for our
    // draw command(s) to finish before swapping the framebuffers
    VkFenceCreateInfo fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
    };
    CALL_VK(vkCreateFence(m_deviceInfo.device, &fenceCreateInfo, nullptr, &m_render.fence))

    // We need to create a semaphore to be able to wait, in the main loop, for our
    // framebuffer to be available for us before drawing.
    VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
    };
    CALL_VK(vkCreateSemaphore(m_deviceInfo.device, &semaphoreCreateInfo, nullptr,
                              &m_render.semaphore))
}

// A helper function
bool VKVideoRendererYUV420::mapMemoryTypeToIndex(uint32_t typeBits, VkFlags requirements_mask,
                                                 uint32_t *typeIndex) const {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_deviceInfo.physicalDevice, &memoryProperties);
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < 32; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) ==
                requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    return false;
}

void VKVideoRendererYUV420::updateUniformBuffers() {
    mat4f_load_rotate_mat(m_ubo.rotation, m_rotation);

    mat4f_load_scale_mat(m_ubo.scale, m_rotation, m_surfaceWidth, m_surfaceHeight,
                         m_frameWidth, m_frameHeight, m_mirror, false);
}

void VKVideoRendererYUV420::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandPoolCreateInfo cmdPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_deviceInfo.queueFamilyIndex,
    };

    VkCommandPool cmdPool;
    CALL_VK(vkCreateCommandPool(m_deviceInfo.device, &cmdPoolCreateInfo, nullptr, &cmdPool))

    VkCommandBuffer cmdBuffer;
    const VkCommandBufferAllocateInfo cmd = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = cmdPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };

    CALL_VK(vkAllocateCommandBuffers(m_deviceInfo.device, &cmd, &cmdBuffer))
    VkCommandBufferBeginInfo cmdBufferInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr};
    CALL_VK(vkBeginCommandBuffer(cmdBuffer, &cmdBufferInfo))

    VkBufferCopy copyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
    };
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    CALL_VK(vkEndCommandBuffer(cmdBuffer))
    VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
    };
    VkFence fence;
    CALL_VK(vkCreateFence(m_deviceInfo.device, &fenceInfo, nullptr, &fence))

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdBuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
    };
    CALL_VK(vkQueueSubmit(m_deviceInfo.queue, 1, &submitInfo, fence) != VK_SUCCESS)
    CALL_VK(vkWaitForFences(m_deviceInfo.device, 1, &fence, VK_TRUE, 100000000) != VK_SUCCESS)
    vkDestroyFence(m_deviceInfo.device, fence, nullptr);

    vkFreeCommandBuffers(m_deviceInfo.device, cmdPool, 1, &cmdBuffer);
    vkDestroyCommandPool(m_deviceInfo.device, cmdPool, nullptr);
}

void VKVideoRendererYUV420::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                         VkMemoryPropertyFlags properties, VkBuffer &buffer,
                                         VkDeviceMemory &bufferMemory) {
    // Create a buffer
    VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &m_deviceInfo.queueFamilyIndex,
    };

    CALL_VK(vkCreateBuffer(m_deviceInfo.device, &bufferInfo, nullptr, &buffer))

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(m_deviceInfo.device, buffer, &memReq);

    VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memReq.size,
            .memoryTypeIndex = 0,  // Memory type assigned in the next step
    };

    // Assign the proper memory type for that buffer
    mapMemoryTypeToIndex(memReq.memoryTypeBits, properties, &allocInfo.memoryTypeIndex);

    // Allocate memory for the buffer
    CALL_VK(vkAllocateMemory(m_deviceInfo.device, &allocInfo, nullptr, &bufferMemory))
    CALL_VK(vkBindBufferMemory(m_deviceInfo.device, buffer, bufferMemory, 0))
}

void VKVideoRendererYUV420::createUniformBuffers() {
    updateUniformBuffers();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkDeviceSize bufferSize = sizeof(m_ubo);

    // Create Vertex shader uniform buffer
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data = nullptr;
    CALL_VK(vkMapMemory(m_deviceInfo.device, stagingBufferMemory, 0, bufferSize, 0, &data))
    memcpy(data, &m_ubo, bufferSize);
    vkUnmapMemory(m_deviceInfo.device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_buffers.uboBuffer, m_buffers.uboBufferMemory);

    copyBuffer(stagingBuffer, m_buffers.uboBuffer, bufferSize);

    vkDestroyBuffer(m_deviceInfo.device, stagingBuffer, nullptr);
    vkFreeMemory(m_deviceInfo.device, stagingBufferMemory, nullptr);
}

void VKVideoRendererYUV420::createVertexBuffer() {
    const Vertex vertices[4]{
            {{1.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
            {{-1.0f, 1.0f,  0.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{1.0f,  -1.0f, 0.0f}, {1.0f, 0.0f}}
    };

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkDeviceSize bufferSize = sizeof(vertices);
    // Create a vertex buffer
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data = nullptr;
    CALL_VK(vkMapMemory(m_deviceInfo.device, stagingBufferMemory, 0, bufferSize, 0, &data))
    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(m_deviceInfo.device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_buffers.vertexBuffer, m_buffers.vertexBufferMemory);

    copyBuffer(stagingBuffer, m_buffers.vertexBuffer, bufferSize);

    vkDestroyBuffer(m_deviceInfo.device, stagingBuffer, nullptr);
    vkFreeMemory(m_deviceInfo.device, stagingBufferMemory, nullptr);
}

// Create our vertex buffer
void VKVideoRendererYUV420::createIndexBuffer() {
    const uint16_t indices[6]{
            0, 1, 2, 2, 3, 0
    };

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    m_indexCount = sizeof(indices) / sizeof(indices[0]);
    VkDeviceSize bufferSize = sizeof(indices);

    // Create a index buffer
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data = nullptr;
    CALL_VK(vkMapMemory(m_deviceInfo.device, stagingBufferMemory, 0, bufferSize, 0, &data))
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(m_deviceInfo.device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_buffers.indexBuffer, m_buffers.indexBufferMemory);

    copyBuffer(stagingBuffer, m_buffers.indexBuffer, bufferSize);

    vkDestroyBuffer(m_deviceInfo.device, stagingBuffer, nullptr);
    vkFreeMemory(m_deviceInfo.device, stagingBufferMemory, nullptr);
}

void VKVideoRendererYUV420::deleteBuffers() const {
    vkDestroyBuffer(m_deviceInfo.device, m_buffers.vertexBuffer, nullptr);
    vkFreeMemory(m_deviceInfo.device, m_buffers.vertexBufferMemory, nullptr);

    vkDestroyBuffer(m_deviceInfo.device, m_buffers.indexBuffer, nullptr);
    vkFreeMemory(m_deviceInfo.device, m_buffers.indexBufferMemory, nullptr);
}

void VKVideoRendererYUV420::deleteUniformBuffers() const {
    vkDestroyBuffer(m_deviceInfo.device, m_buffers.uboBuffer, nullptr);
    vkFreeMemory(m_deviceInfo.device, m_buffers.uboBufferMemory, nullptr);
}

bool VKVideoRendererYUV420::isInitialized() const {
    return m_deviceInfo.initialized;
}

// A help function to map required memory property into a VK memory type
// memory type is an index into the array of 32 entries; or the bit index
// for the memory type ( each BIT of an 32 bit integer is a type ).
VkResult VKVideoRendererYUV420::allocateMemoryTypeFromProperties(uint32_t typeBits,
                                                                 VkFlags requirements_mask,
                                                                 uint32_t *typeIndex) {
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < 32; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((m_deviceInfo.memoryProperties.memoryTypes[i].propertyFlags &
                 requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return VK_SUCCESS;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return VK_ERROR_MEMORY_MAP_FAILED;
}

size_t VKVideoRendererYUV420::getBufferOffset(VulkanTexture *texture, TextureType type,
                                              size_t width, size_t height) {
    size_t offset = 0;
    if (type == tTexY) {
        texture->width = width;
        texture->height = height;
    } else if (type == tTexU) {
        texture->width = width / 2;
        texture->height = height / 2;
        offset = width * height;
    } else if (type == tTexV) {
        texture->width = width / 2;
        texture->height = height / 2;
        offset = width * height * 5 / 4;
    }

    return offset;
}

void VKVideoRendererYUV420::copyTextureData(VulkanTexture *texture, uint8_t *data) {
    auto *mappedData = (uint8_t *) texture->mapped;
    for (int i = 0; i < texture->height; ++i) {
        memcpy(mappedData, data, texture->width);
        mappedData += texture->layout.rowPitch;
        data += texture->width;
    }
}

VkResult
VKVideoRendererYUV420::loadTexture(uint8_t *buffer, TextureType type, size_t width, size_t height,
                                   VulkanTexture *texture, VkImageUsageFlags usage,
                                   VkFlags required_props) {
    if (!(usage | required_props)) {
        LOGE("No usage and required_pros");
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    // Check for linear supportability
    VkFormatProperties props;
    bool needBlit = true;
    vkGetPhysicalDeviceFormatProperties(m_deviceInfo.physicalDevice, kTextureFormat, &props);
    assert((props.linearTilingFeatures | props.optimalTilingFeatures) &
           VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

    if (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
        // linear format supporting the required texture
        needBlit = false;
    }

    size_t offset = getBufferOffset(texture, type, width, height);

    // Allocate the linear texture so texture could be copied over
    VkImageCreateInfo imageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = kTextureFormat,
            .extent = {static_cast<uint32_t>(texture->width),
                       static_cast<uint32_t>(texture->height), 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_LINEAR,
            .usage = (needBlit ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                               : VK_IMAGE_USAGE_SAMPLED_BIT),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &m_deviceInfo.queueFamilyIndex,
            .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
    };
    VkMemoryAllocateInfo memAlloc = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = 0,
            .memoryTypeIndex = 0,
    };

    VkMemoryRequirements memReqs;
    CALL_VK(vkCreateImage(m_deviceInfo.device, &imageCreateInfo, nullptr, &texture->image))
    vkGetImageMemoryRequirements(m_deviceInfo.device, texture->image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    VK_CHECK(allocateMemoryTypeFromProperties(memReqs.memoryTypeBits,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                              &memAlloc.memoryTypeIndex))
    CALL_VK(vkAllocateMemory(m_deviceInfo.device, &memAlloc, nullptr, &texture->mem))
    CALL_VK(vkBindImageMemory(m_deviceInfo.device, texture->image, texture->mem, 0))

    if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        const VkImageSubresource subres = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .arrayLayer = 0,
        };
        vkGetImageSubresourceLayout(m_deviceInfo.device, texture->image, &subres, &texture->layout);
        CALL_VK(vkMapMemory(m_deviceInfo.device, texture->mem, 0, memAlloc.allocationSize, 0,
                            &texture->mapped))

        copyTextureData(texture, buffer + offset);
    }

    texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkCommandPoolCreateInfo cmdPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_deviceInfo.queueFamilyIndex,
    };

    VkCommandPool cmdPool;
    CALL_VK(vkCreateCommandPool(m_deviceInfo.device, &cmdPoolCreateInfo, nullptr, &cmdPool))

    VkCommandBuffer gfxCmd;
    const VkCommandBufferAllocateInfo cmd = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = cmdPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };

    CALL_VK(vkAllocateCommandBuffers(m_deviceInfo.device, &cmd, &gfxCmd))
    VkCommandBufferBeginInfo cmdBufferInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr};
    CALL_VK(vkBeginCommandBuffer(gfxCmd, &cmdBufferInfo))

    // If linear is supported, we are done
    VkImage stageImage = VK_NULL_HANDLE;
    VkDeviceMemory stageMem = VK_NULL_HANDLE;
    if (!needBlit) {
        setImageLayout(gfxCmd, texture->image, VK_IMAGE_LAYOUT_PREINITIALIZED,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       VK_PIPELINE_STAGE_HOST_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    } else {
        // save current image and mem as staging image and memory
        stageImage = texture->image;
        stageMem = texture->mem;
        texture->image = VK_NULL_HANDLE;
        texture->mem = VK_NULL_HANDLE;

        // Create a tile texture to blit into
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage =
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        CALL_VK(vkCreateImage(m_deviceInfo.device, &imageCreateInfo, nullptr,
                              &texture->image))
        vkGetImageMemoryRequirements(m_deviceInfo.device, texture->image, &memReqs);

        memAlloc.allocationSize = memReqs.size;
        VK_CHECK(allocateMemoryTypeFromProperties(
                memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                &memAlloc.memoryTypeIndex))
        CALL_VK(vkAllocateMemory(m_deviceInfo.device, &memAlloc, nullptr, &texture->mem))
        CALL_VK(vkBindImageMemory(m_deviceInfo.device, texture->image, texture->mem, 0))

        // transitions image out of UNDEFINED type
        setImageLayout(gfxCmd, stageImage, VK_IMAGE_LAYOUT_PREINITIALIZED,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        setImageLayout(gfxCmd, texture->image, VK_IMAGE_LAYOUT_UNDEFINED,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkImageCopy bltInfo;
        bltInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bltInfo.srcSubresource.mipLevel = 0;
        bltInfo.srcSubresource.baseArrayLayer = 0;
        bltInfo.srcSubresource.layerCount = 1;
        bltInfo.srcOffset.x = 0;
        bltInfo.srcOffset.y = 0;
        bltInfo.srcOffset.z = 0;
        bltInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bltInfo.dstSubresource.mipLevel = 0;
        bltInfo.dstSubresource.baseArrayLayer = 0;
        bltInfo.dstSubresource.layerCount = 1;
        bltInfo.dstOffset.x = 0;
        bltInfo.dstOffset.y = 0;
        bltInfo.dstOffset.z = 0;
        bltInfo.extent.width = static_cast<uint32_t>(texture->width);
        bltInfo.extent.height = static_cast<uint32_t>(texture->height);
        bltInfo.extent.depth = 1;

        vkCmdCopyImage(gfxCmd, stageImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                       &bltInfo);

        setImageLayout(gfxCmd, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }

    CALL_VK(vkEndCommandBuffer(gfxCmd))
    VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
    };
    VkFence fence;
    CALL_VK(vkCreateFence(m_deviceInfo.device, &fenceInfo, nullptr, &fence))

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &gfxCmd,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
    };
    CALL_VK(vkQueueSubmit(m_deviceInfo.queue, 1, &submitInfo, fence) != VK_SUCCESS)
    CALL_VK(vkWaitForFences(m_deviceInfo.device, 1, &fence, VK_TRUE, 100000000) != VK_SUCCESS)
    vkDestroyFence(m_deviceInfo.device, fence, nullptr);

    vkFreeCommandBuffers(m_deviceInfo.device, cmdPool, 1, &gfxCmd);
    vkDestroyCommandPool(m_deviceInfo.device, cmdPool, nullptr);
    if (stageImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_deviceInfo.device, stageImage, nullptr);
        vkFreeMemory(m_deviceInfo.device, stageMem, nullptr);
    }
    return VK_SUCCESS;
}

void VKVideoRendererYUV420::createRenderPass() {
    // Create render pass
    VkAttachmentDescription attachmentDescriptions{
            .format = m_swapchainInfo.displayFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference colourReference{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpassDescription{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colourReference,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
    };
    VkRenderPassCreateInfo renderPassCreateInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .attachmentCount = 1,
            .pAttachments = &attachmentDescriptions,
            .subpassCount = 1,
            .pSubpasses = &subpassDescription,
            .dependencyCount = 0,
            .pDependencies = nullptr,
    };
    CALL_VK(vkCreateRenderPass(m_deviceInfo.device, &renderPassCreateInfo, nullptr,
                               &m_render.renderPass))
}
