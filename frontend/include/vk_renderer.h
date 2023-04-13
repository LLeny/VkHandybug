#pragma once

#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "global.h"
#include "log.h"
#include "VkBootstrap.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "vk_initializers.h"
#include "vk_texture.h"
#include "vk_mem_alloc.h"
#include "system.h"
#include "cart.h"
#include "gui.h"
#include <vulkan/vulkan.h>

#define VK_CHECK(x)                                                  \
    do                                                               \
    {                                                                \
        VkResult err = x;                                            \
        if (err)                                                     \
        {                                                            \
            LOG(LOG_ERROR) << "Detected Vulkan error: " << (int)err; \
            abort();                                                 \
        }                                                            \
    } while (0)

typedef struct LynxScreenBuffer
{
    uint8_t buffer[HANDY_SCREEN_WIDTH * HANDY_SCREEN_HEIGHT / 2];
    uint8_t palette[32];
    uint8_t rotation;
} LynxScreenBuffer;

typedef struct VkTextureView
{
    int id = 0;
    bool ready = false;
    int groupXDiv = 16;
    int groupYDiv = 6;

    VmaAllocation allocation{};
    VulkanTexture texture{};
    VmaAllocationInfo allocationInfo{};
    AllocatedBuffer buffer{};
    VkCommandBuffer commandBuffer{};
    VkDescriptorSet descriptorSet{};
    VkPipelineLayout pipelineLayout{};
    VkPipeline pipeline{};
} VktextureView;

class VulkanRenderer
{
  public:
    VulkanRenderer();
    ~VulkanRenderer();
    void initialize();
    void terminate();
    int64_t render(std::shared_ptr<GUI> gui);
    bool should_close();
    void setTitle(std::string title);
    ImVec2 get_dimensions();
    ImVec2 get_position();
    void set_dimensions(ImVec2 dim);
    void set_position(ImVec2 pos);

    void register_file_open_callback(std::function<void(std::string)> callback);
    void register_key_event_callback(std::function<void(int, int, bool)> callback);
    void set_rotation(int screenid, uint8_t rotation);
    ImTextureID get_texture_ID(int viewId);
    bool delete_view(int view);
    int add_screen_view(uint16_t baseAddress);
    void render_screen_view(int id, uint8_t *screen, uint8_t *palette);

  private:
    GLFWwindow *_mainWindow;
    ImGui_ImplVulkanH_Window _mainWindowData;
    ImVec2 _dimensions{};

    int _minImageCount = 2;
    int _viewId = 0;
    bool _swapChainRebuild = false;
    int _fontWidth = 8;
    int _fontHeight = 16;

    uint8_t _rotation{};

    std::vector<VkTextureView> _views{};

    vkb::Device _vkbDevice;
    VkInstance _instance = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties _physicalDeviceProperties{};
    VkDevice _device = VK_NULL_HANDLE;
    uint32_t _queueFamily = (uint32_t)-1;
    VkQueue _queue = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debugReport = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
    VkPipelineCache _pipelineCache = VK_NULL_HANDLE;
    VkCommandPool _commandPool = VK_NULL_HANDLE;

    VmaAllocator _allocator{};
    VkAllocationCallbacks *_allocationCallbacks{};

    std::vector<VkShaderModule> _shaderModules;

    std::function<void(std::string)> _fileDropCallback{};
    std::function<void(int, int, bool)> _keyEventCallback{};

    struct Compute
    {
        VkQueue queue;
        uint32_t queueFamily;
        VkCommandPool commandPool;
        VkSemaphore semaphore;
        VkDescriptorSetLayout screenViewDescriptorSetLayout;
        VkPipelineShaderStageCreateInfo screenViewShader;
    } _compute;

#if defined(VKB_DEBUG)
    VkDebugUtilsMessengerEXT debugUtilsMessenger{VK_NULL_HANDLE};
    VkDebugReportCallbackEXT debugReportCallback{VK_NULL_HANDLE};
#endif

    void setup_vulkan(const char **extensions, uint32_t extensions_count);
    void setup_vulkan_window(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height);
    void cleanup_vulkan_window();
    void cleanup_vulkan();

    VkCommandBuffer create_command_buffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin);
    void flush_command_buffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free);
    void set_image_layout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    void prepare_compute();
    void destroy_compute();

    void prepare_view_compute(VkTextureView &view, size_t bufferSize, VkPipelineShaderStageCreateInfo &shader, VkDescriptorSetLayout &descLayout);
    void destroy_view_compute(VkTextureView &view);

    void destroy_view_texture(VkTextureView &view);

    void build_compute_command_buffer(VkTextureView &view);

    VkPipelineShaderStageCreateInfo load_shader(std::string fileName, VkShaderStageFlagBits stage);

    void frame_render(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data);
    void frame_present(ImGui_ImplVulkanH_Window *wd);
    void render_ImGui(std::shared_ptr<GUI> gui);

    void prepare_view_texture(VkTextureView &view, VkFormat format);
};