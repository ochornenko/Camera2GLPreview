#include <__bit_reference>

#ifndef _VK_VIDEO_RENDERER_YUV_H_
#define _VK_VIDEO_RENDERER_YUV_H_

#include "VideoRenderer.h"
#include <vulkan/vulkan.h>

class VKVideoRendererYUV420 : public VideoRenderer {
public:
    VKVideoRendererYUV420();

    ~VKVideoRendererYUV420() override;

    void
    init(ANativeWindow *window, AAssetManager *assetManager, size_t width, size_t height) override;

    void render() override;

    void draw(uint8_t *buffer, size_t length, size_t width, size_t height, float rotation, bool mirror) override;

    void setParameters(uint32_t params) override;

    uint32_t getParameters() override;

    int createProgram(const char *pVertexSource, const char *pFragmentSource) override;

private:
    enum TextureType {
        tTexY, tTexU, tTexV
    };

    struct Vertex {
        float pos[3];
        float uv[2];
    };

    struct UniformBufferObject {
        float rotation[16];
        float scale[16];
    };

    UniformBufferObject m_ubo{};

    struct VulkanTexture {
        VkSampler sampler;
        VkImage image;
        __unused VkImageLayout imageLayout;
        VkSubresourceLayout layout;
        VkDeviceMemory mem;
        VkImageView view;
        size_t width;
        size_t height;
        void *mapped;
    };

    struct VulkanDeviceInfo {
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        VkDevice device;
        uint32_t queueFamilyIndex;

        VkSurfaceKHR surface;
        VkQueue queue;

        bool initialized;
    };
    VulkanDeviceInfo m_deviceInfo{};

    struct VulkanSwapchainInfo {
        VkSwapchainKHR swapchain;
        uint32_t swapchainLength;

        VkExtent2D displaySize;
        VkFormat displayFormat;

        // array of frame buffers and views
        std::unique_ptr<VkFramebuffer[]> framebuffers;
        std::unique_ptr<VkImage[]> displayImages;
        std::unique_ptr<VkImageView[]> displayViews;
    };
    VulkanSwapchainInfo m_swapchainInfo;

    struct VulkanRenderInfo {
        VkRenderPass renderPass;
        VkCommandPool cmdPool;
        std::unique_ptr<VkCommandBuffer[]> cmdBuffer;
        uint32_t cmdBufferLen;
        VkSemaphore semaphore;
        VkFence fence;
    };
    VulkanRenderInfo m_render;

    struct VulkanGfxPipelineInfo {
        VkDescriptorSetLayout descLayout;
        VkDescriptorPool descPool;
        VkDescriptorSet descSet;
        VkPipelineLayout layout;
        VkPipelineCache cache;
        VkPipeline pipeline;
    };
    VulkanGfxPipelineInfo m_gfxPipeline{};

    struct VulkanBufferInfo {
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;

        VkBuffer uboBuffer;
        VkDeviceMemory uboBufferMemory;
    };
    VulkanBufferInfo m_buffers{};

    static const uint32_t kTextureCount = 3;
    static const VkFormat kTextureFormat = VK_FORMAT_R8_UNORM;
    const TextureType texType[kTextureCount];
    struct VulkanTexture textures[kTextureCount]{};

    uint8_t *m_pBuffer;
    uint32_t m_indexCount;

    AAssetManager *m_assetManager;

    void createDevice(ANativeWindow *platformWindow, VkApplicationInfo *appInfo);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer &buffer, VkDeviceMemory &bufferMemory);

    void createRenderPipeline();

    void createDescriptorSet();

    VkResult createGraphicsPipeline();

    void createFrameBuffers(VkImageView depthView = VK_NULL_HANDLE);

    void createRenderPass();

    void createSwapChain();

    void createUniformBuffers();

    void createVertexBuffer();

    void createIndexBuffer();

    void createCommandPool();

    bool createTextures();

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    static void copyTextureData(VulkanTexture *texture, uint8_t *data);

    void updateDescriptorSet();

    void updateUniformBuffers();

    bool updateTextures();

    bool
    mapMemoryTypeToIndex(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) const;

    void deleteSwapChain() const;

    void deleteCommandPool() const;

    void deleteRenderPass() const;

    void deleteGraphicsPipeline();

    void deleteTextures() const;

    void deleteBuffers() const;

    void deleteUniformBuffers() const;

    bool isInitialized() const;

    VkResult allocateMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask,
                                              uint32_t *typeIndex);

    static size_t
    getBufferOffset(VulkanTexture *texture, TextureType type, size_t width, size_t height);

    static void setImageLayout(VkCommandBuffer cmdBuffer,
                               VkImage image,
                               VkImageLayout oldImageLayout,
                               VkImageLayout newImageLayout,
                               VkPipelineStageFlags srcStages,
                               VkPipelineStageFlags destStages);

    VkResult loadTexture(uint8_t *buffer, TextureType type, size_t width, size_t height,
                         VulkanTexture *texture, VkImageUsageFlags usage, VkFlags required_props);
};

#endif //_VK_VIDEO_RENDERER_YUV_H_
