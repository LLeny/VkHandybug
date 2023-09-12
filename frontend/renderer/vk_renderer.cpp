#include "vk_renderer.h"
#include "gui.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "config.h"

#if defined(VKB_DEBUG)

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        LOG(LOGLEVEL_ERROR) << messageType << ": " << pCallbackData->pMessage;
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        LOG(LOGLEVEL_WARN) << messageType << ": " << pCallbackData->pMessage;
    }
    else
    {
        LOG(LOGLEVEL_INFO) << messageType << ": " << pCallbackData->pMessage;
    }
    return VK_FALSE;
}
#endif

VulkanRenderer::VulkanRenderer()
{
}

VulkanRenderer::~VulkanRenderer()
{
    terminate();
}

void VulkanRenderer::setTitle(std::string title)
{
    glfwSetWindowTitle(_mainWindow, title.c_str());
}

void VulkanRenderer::setup_vulkan(const char **extensions, uint32_t extensions_count)
{
    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name(APP_NAME)
                        .request_validation_layers(true)
#if defined(VKB_DEBUG)
                        .enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
                        .use_default_debug_messenger()
                        .add_debug_messenger_type(VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
                        .add_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                        .set_debug_callback(debug_callback)
                        .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT)
                        .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT)
#endif
                        .require_api_version(1, 0, 0)
                        .build();

    if (!inst_ret)
    {
        LOG(LOGLEVEL_ERROR) << "Failed to create Vulkan instance. Error: " << inst_ret.error().message();
        exit(1);
    }

    auto vkb_inst = inst_ret.value();

    _instance = vkb_inst.instance;
    _debugReport = vkb_inst.debug_messenger;

    VK_CHECK(glfwCreateWindowSurface(_instance, _mainWindow, _allocationCallbacks, &_surface));

    vkb::PhysicalDeviceSelector selector{vkb_inst};
    auto phys_ret = selector.set_minimum_version(1, 0).set_surface(_surface).select();
    if (!phys_ret)
    {
        LOG(LOGLEVEL_ERROR) << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message();
        exit(1);
    }
    vkb::PhysicalDevice physicalDevice = phys_ret.value();

    vkb::DeviceBuilder deviceBuilder{physicalDevice};
    auto device_ret = deviceBuilder.build();
    if (!device_ret)
    {
        LOG(LOGLEVEL_ERROR) << "Failed to create Vulkan device. Error: " << device_ret.error().message() << "\n";
        exit(1);
    }
    _vkbDevice = device_ret.value();

    _device = _vkbDevice.device;
    _physicalDevice = physicalDevice.physical_device;

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(_physicalDevice, &props);

    LOG(LOGLEVEL_INFO) << "Will be using Vulkan device: " << props.deviceName;

    auto qr = _vkbDevice.get_queue(vkb::QueueType::graphics);
    if (!qr)
    {
        LOG(LOGLEVEL_ERROR) << "Failed to get graphics queue. Error: " << qr.error().message() << "\n";
        exit(1);
    }
    _queue = qr.value();
    _queueFamily = _vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    auto qrc = _vkbDevice.get_queue(vkb::QueueType::compute);
    if (!qrc)
    {
        LOG(LOGLEVEL_DEBUG) << "Failed to get compute queue. Error: " << qrc.error().message() << "\n";
        _compute.queue = _queue;
        _compute.queueFamily = _queueFamily;
    }
    else
    {
        _compute.queue = qrc.value();
        _compute.queueFamily = _vkbDevice.get_queue_index(vkb::QueueType::compute).value();
    }

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _physicalDevice;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    vmaCreateAllocator(&allocatorInfo, &_allocator);

    vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);

    {
        VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 10},
                                             {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10},
                                             {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 10},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 10},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10},
                                             {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10}};
        auto pool_info = vkinit::descriptor_pool_create_info(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, 1000 * IM_ARRAYSIZE(pool_sizes), (uint32_t)IM_ARRAYSIZE(pool_sizes), pool_sizes);
        VK_CHECK(vkCreateDescriptorPool(_device, &pool_info, _allocationCallbacks, &_descriptorPool));
    }

    VkCommandPoolCreateInfo cmdPoolInfo = vkinit::command_pool_create_info(_queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VK_CHECK(vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_commandPool));

    _compute.screenViewShader = load_shader("shaders/lynx_render.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);

    prepare_compute();
}

void VulkanRenderer::setup_vulkan_window(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height)
{
    wd->Surface = surface;

    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, _queueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
        LOG(LOGLEVEL_ERROR) << "Error no WSI support on physical device 0";
        exit(-1);
    }

    const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(_physicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(_physicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));

    IM_ASSERT(_minImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(_instance, _physicalDevice, _device, wd, _queueFamily, _allocationCallbacks, width, height, _minImageCount);
}

void VulkanRenderer::cleanup_vulkan()
{
    vkDeviceWaitIdle(_device);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyDevice(_device, nullptr);
    vkb::destroy_debug_utils_messenger(_instance, _debugReport);
    vkDestroyInstance(_instance, nullptr);
}

void VulkanRenderer::cleanup_vulkan_window()
{
    ImGui_ImplVulkanH_DestroyWindow(_instance, _device, &_mainWindowData, _allocationCallbacks);
}

void VulkanRenderer::frame_render(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data)
{
    if (_swapChainRebuild)
    {
        return;
    }
    VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkResult err = vkAcquireNextImageKHR(_device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        _swapChainRebuild = true;
        return;
    }

    ImGui_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
    VK_CHECK(vkWaitForFences(_device, 1, &fd->Fence, VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(_device, 1, &fd->Fence));

    VK_CHECK(vkResetCommandPool(_device, fd->CommandPool, 0));

    auto cmdinfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(fd->CommandBuffer, &cmdinfo));

    auto renderpassinfo = vkinit::renderpass_begin_info(wd->RenderPass, {(unsigned int)wd->Width, (unsigned int)wd->Height}, fd->Framebuffer);
    renderpassinfo.pClearValues = &wd->ClearValue;
    vkCmdBeginRenderPass(fd->CommandBuffer, &renderpassinfo, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    vkCmdEndRenderPass(fd->CommandBuffer);

    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    std::vector<VkCommandBuffer> cmdbuffers{};

    for (auto &view : _views)
    {
        if (!view.ready)
        {
            continue;
        }
        cmdbuffers.push_back(view.commandBuffer);
    }

    VkSubmitInfo computeSubmitInfo = vkinit::submit_info(cmdbuffers.data());
    computeSubmitInfo.commandBufferCount = cmdbuffers.size();
    computeSubmitInfo.pWaitSemaphores = &image_acquired_semaphore;
    computeSubmitInfo.waitSemaphoreCount = 1;
    computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
    computeSubmitInfo.signalSemaphoreCount = 1;
    computeSubmitInfo.pSignalSemaphores = &_compute.semaphore;
    VK_CHECK(vkQueueSubmit(_compute.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(_compute.queue));

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    auto submitinfo = vkinit::submit_info(&fd->CommandBuffer);
    submitinfo.waitSemaphoreCount = 1;
    submitinfo.pWaitSemaphores = &_compute.semaphore;
    submitinfo.pWaitDstStageMask = &wait_stage;
    submitinfo.signalSemaphoreCount = 1;
    submitinfo.pSignalSemaphores = &render_complete_semaphore;

    VK_CHECK(vkEndCommandBuffer(fd->CommandBuffer));
    VK_CHECK(vkQueueSubmit(_queue, 1, &submitinfo, fd->Fence));
}

void VulkanRenderer::frame_present(ImGui_ImplVulkanH_Window *wd)
{
    if (_swapChainRebuild)
    {
        return;
    }
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    auto info = vkinit::present_info();
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(_queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        _swapChainRebuild = true;
        return;
    }
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount;
}

ImVec2 VulkanRenderer::get_dimensions()
{
    return _dimensions;
}

ImVec2 VulkanRenderer::get_position()
{
    int xpos, ypos;
    glfwGetWindowPos(_mainWindow, &xpos, &ypos);
    return {(float)xpos, (float)ypos};
}

void VulkanRenderer::set_dimensions(ImVec2 dim)
{
    glfwSetWindowSize(_mainWindow, dim.x, dim.y);
}

void VulkanRenderer::set_position(ImVec2 pos)
{
    glfwSetWindowPos(_mainWindow, pos.x, pos.y);
}

void VulkanRenderer::initialize()
{
    glfwSetErrorCallback([](int error, const char *description) { LOG(LOGLEVEL_ERROR) << "GLFW Error " << error << ": " << description; });

    if (!glfwInit())
    {
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    _mainWindow = glfwCreateWindow(1280, 720, "VulkanRenderer", NULL, NULL);
    if (!glfwVulkanSupported())
    {
        LOG(LOGLEVEL_ERROR) << "GLFW: Vulkan Not Supported";
        return;
    }

    glfwSetWindowUserPointer(_mainWindow, this);

    glfwSetDropCallback(_mainWindow, [](GLFWwindow *window, int count, const char **paths) {
        if (count != 1)
        {
            return;
        }

        auto self = static_cast<VulkanRenderer *>(glfwGetWindowUserPointer(window));
        if (!self->_fileDropCallback)
        {
            return;
        }

        self->_fileDropCallback(paths[0]);
    });

    glfwSetFramebufferSizeCallback(_mainWindow, [](GLFWwindow *window, int width, int height) {
        auto self = static_cast<VulkanRenderer *>(glfwGetWindowUserPointer(window));
        self->_dimensions = {static_cast<float>(width), static_cast<float>(height)};
    });

    glfwSetKeyCallback(_mainWindow, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto self = static_cast<VulkanRenderer *>(glfwGetWindowUserPointer(window));
        if (!self->_keyEventCallback)
        {
            return;
        }

        // LOG(LOGLEVEL_DEBUG) << "Renderer - key: " << key << " scancode: " << scancode << " action: " << action << " mods: " << mods;

        switch (action)
        {
        case GLFW_PRESS:
            self->_keyEventCallback(key, mods, true);
            break;
        case GLFW_RELEASE:
            self->_keyEventCallback(key, mods, false);
            break;
        default:
            break;
        }
    });

    uint32_t extensions_count = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    setup_vulkan(extensions, extensions_count);

    int w, h;
    glfwGetFramebufferSize(_mainWindow, &w, &h);

    _dimensions = {static_cast<float>(w), static_cast<float>(h)};

    ImGui_ImplVulkanH_Window *wd = &_mainWindowData;
    setup_vulkan_window(wd, _surface, w, h);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = Config::get_instance().imgui_ini();

    ImGui_ImplGlfw_InitForVulkan(_mainWindow, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = _instance;
    init_info.PhysicalDevice = _physicalDevice;
    init_info.Device = _device;
    init_info.QueueFamily = _queueFamily;
    init_info.Queue = _queue;
    init_info.PipelineCache = _pipelineCache;
    init_info.DescriptorPool = _descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = _minImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = _allocationCallbacks;
    ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

    float xscale, yscale;
    glfwGetWindowContentScale(_mainWindow, &xscale, &yscale);

    Config::get_instance().apply_font(yscale);

    VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
    VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

    VK_CHECK(vkResetCommandPool(_device, command_pool, 0));

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &command_buffer;
    VK_CHECK(vkEndCommandBuffer(command_buffer));
    VK_CHECK(vkQueueSubmit(_queue, 1, &end_info, VK_NULL_HANDLE));

    VK_CHECK(vkDeviceWaitIdle(_device));
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanRenderer::terminate()
{
    VK_CHECK(vkDeviceWaitIdle(_device));

    for (auto &view : _views)
    {
        destroy_view_texture(view);
        destroy_view_compute(view);
    }

    _swapChainRebuild = true;

    destroy_compute();

    for (auto &shaderModule : _shaderModules)
    {
        vkDestroyShaderModule(_device, shaderModule, nullptr);
    }

    vmaDestroyAllocator(_allocator);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    cleanup_vulkan_window();
    cleanup_vulkan();

    glfwDestroyWindow(_mainWindow);
    glfwTerminate();
}

int64_t VulkanRenderer::render(std::shared_ptr<GUI> gui)
{
    double time = glfwGetTime();
    render_ImGui(gui);
    return (glfwGetTime() - time) * (1000000000);
}

VkPipelineShaderStageCreateInfo VulkanRenderer::load_shader(std::string fileName, VkShaderStageFlagBits stage)
{
    std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

    if (!is.is_open())
    {
        LOG(LOGLEVEL_ERROR) << "Error: Could not open shader file \"" << fileName << "\"";
        abort();
    }

    size_t size = is.tellg();
    is.seekg(0, std::ios::beg);
    char *shaderCode = new char[size];
    is.read(shaderCode, size);
    is.close();

    assert(size > 0);

    VkShaderModule shaderModule;
    VkShaderModuleCreateInfo moduleCreateInfo{};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = (uint32_t *)shaderCode;

    VK_CHECK(vkCreateShaderModule(_device, &moduleCreateInfo, NULL, &shaderModule));

    delete[] shaderCode;

    auto shaderStage = vkinit::pipeline_shader_stage_create_info(stage, shaderModule);
    assert(shaderStage.module != VK_NULL_HANDLE);

    _shaderModules.push_back(shaderStage.module);

    return shaderStage;
}

void VulkanRenderer::build_compute_command_buffer(VkTextureView &view)
{
    vkQueueWaitIdle(_compute.queue);

    auto cmdBufAllocateInfo = vkinit::command_buffer_allocate_info(_compute.commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdBufAllocateInfo, &view.commandBuffer));

    auto cmdBufInfo = vkinit::command_buffer_begin_info();

    VK_CHECK(vkBeginCommandBuffer(view.commandBuffer, &cmdBufInfo));

    vkCmdBindPipeline(view.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, view.pipeline);
    vkCmdBindDescriptorSets(view.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, view.pipelineLayout, 0, 1, &view.descriptorSet, 0, 0);

    vkCmdDispatch(view.commandBuffer, view.texture.width / view.groupXDiv, view.texture.height / view.groupYDiv, 1);

    vkEndCommandBuffer(view.commandBuffer);

    view.ready = true;
}

void VulkanRenderer::prepare_view_compute(VkTextureView &view, size_t bufferSize, VkPipelineShaderStageCreateInfo &shader, VkDescriptorSetLayout &descLayout)
{
    auto pPipelineLayoutCreateInfo = vkinit::pipeline_layout_create_info();
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descLayout;
    VK_CHECK(vkCreatePipelineLayout(_device, &pPipelineLayoutCreateInfo, nullptr, &view.pipelineLayout));

    auto allocInfo = vkinit::descriptorset_allocate_info(_descriptorPool, &descLayout, 1);
    VK_CHECK(vkAllocateDescriptorSets(_device, &allocInfo, &view.descriptorSet));

    VkBufferCreateInfo screenbufferInfo{};
    screenbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    screenbufferInfo.pNext = nullptr;
    screenbufferInfo.size = bufferSize;
    screenbufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo screenvmaallocInfo{};
    screenvmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    screenvmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VK_CHECK(vmaCreateBuffer(_allocator, &screenbufferInfo, &screenvmaallocInfo, &view.buffer._buffer, &view.buffer._allocation, &view.allocationInfo));

    VkDescriptorBufferInfo screenbinfo;
    screenbinfo.buffer = view.buffer._buffer;
    screenbinfo.offset = 0;
    screenbinfo.range = bufferSize;

    auto descBuff = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, view.descriptorSet, &screenbinfo, 0);
    auto descImages = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, view.descriptorSet, &view.texture.descriptor, 1);

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets{descBuff, descImages};

    vkUpdateDescriptorSets(_device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

    auto computePipelineCreateInfo = vkinit::computepipeline_create_info(view.pipelineLayout, 0);

    computePipelineCreateInfo.stage = shader;

    VK_CHECK(vkCreateComputePipelines(_device, _pipelineCache, 1, &computePipelineCreateInfo, nullptr, &view.pipeline));

    build_compute_command_buffer(view);
}

void VulkanRenderer::prepare_compute()
{
    std::vector<VkDescriptorSetLayoutBinding> screenViewSetLayoutBindings =
        {
            vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
            vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
        };

    auto screenViewDescriptorLayout = vkinit::descriptorset_layout_create_info(screenViewSetLayoutBindings);
    VK_CHECK(vkCreateDescriptorSetLayout(_device, &screenViewDescriptorLayout, nullptr, &_compute.screenViewDescriptorSetLayout));

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = _compute.queueFamily;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_compute.commandPool));

    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_compute.semaphore));
}

void VulkanRenderer::destroy_view_compute(VkTextureView &view)
{
    view.ready = false;
    vkFreeCommandBuffers(_device, _compute.commandPool, 1, &view.commandBuffer);
    vmaDestroyBuffer(_allocator, view.buffer._buffer, view.buffer._allocation);
    vkDestroyPipelineLayout(_device, view.pipelineLayout, nullptr);
    vkFreeDescriptorSets(_device, _descriptorPool, 1, &view.descriptorSet);
}

void VulkanRenderer::destroy_compute()
{
    vkDestroySemaphore(_device, _compute.semaphore, nullptr);
    vkDestroyDescriptorSetLayout(_device, _compute.screenViewDescriptorSetLayout, nullptr);
    vkDestroyCommandPool(_device, _compute.commandPool, nullptr);
}

VkCommandBuffer VulkanRenderer::create_command_buffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
{
    auto cmdBufAllocateInfo = vkinit::command_buffer_allocate_info(pool, 1, level);

    VkCommandBuffer cmdBuffer;
    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdBufAllocateInfo, &cmdBuffer));
    if (begin)
    {
        auto cmdBufInfo = vkinit::command_buffer_begin_info();
        VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
    }
    return cmdBuffer;
}

void VulkanRenderer::flush_command_buffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
{
    if (commandBuffer == VK_NULL_HANDLE)
    {
        return;
    }

    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = vkinit::submit_info(&commandBuffer);
    VkFenceCreateInfo fenceInfo = vkinit::fence_create_info();
    VkFence fence;
    VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &fence));
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));
    VK_CHECK(vkWaitForFences(_device, 1, &fence, VK_TRUE, 1000000000));
    vkDestroyFence(_device, fence, nullptr);
    if (free)
    {
        vkFreeCommandBuffers(_device, pool, 1, &commandBuffer);
    }
}

void VulkanRenderer::set_image_layout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    VkImageMemoryBarrier imageMemoryBarrier = vkinit::image_memory_barrier(oldImageLayout, newImageLayout, image);
    imageMemoryBarrier.subresourceRange = subresourceRange;

    switch (oldImageLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        imageMemoryBarrier.srcAccessMask = 0;
        break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        break;
    }

    switch (newImageLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        if (imageMemoryBarrier.srcAccessMask == 0)
        {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        break;
    }

    vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

bool VulkanRenderer::should_close()
{
    return glfwWindowShouldClose(_mainWindow);
}

void VulkanRenderer::render_ImGui(std::shared_ptr<GUI> ui)
{
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glfwPollEvents();

    if (_swapChainRebuild)
    {
        int width, height;
        glfwGetFramebufferSize(_mainWindow, &width, &height);
        if (width > 0 && height > 0)
        {
            ImGui_ImplVulkan_SetMinImageCount(_minImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(_instance, _physicalDevice, _device, &_mainWindowData, _queueFamily, _allocationCallbacks, width, height, _minImageCount);
            _mainWindowData.FrameIndex = 0;
            _swapChainRebuild = false;
        }
    }

    if (ui->menu().settings().update_pending())
    {
        ui->menu().settings().set_update_pending(false);
        Config::get_instance().update_ui_settings();
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    auto id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    ui->render(id);

    ImGui::Render();
    ImDrawData *draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (!is_minimized)
    {
        ImGui_ImplVulkanH_Window *wd = &_mainWindowData;
        wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
        wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
        wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
        wd->ClearValue.color.float32[3] = clear_color.w;
        frame_render(wd, draw_data);
        frame_present(wd);
    }
}

void VulkanRenderer::register_file_open_callback(std::function<void(std::string)> callback)
{
    _fileDropCallback = std::move(callback);
}

void VulkanRenderer::register_key_event_callback(std::function<void(int, int, bool)> callback)
{
    _keyEventCallback = std::move(callback);
}

void VulkanRenderer::set_rotation(int screen_id, uint8_t rotation)
{
    _rotation = rotation;

    vkQueueWaitIdle(_compute.queue);

    auto found = std::find_if(_views.begin(), _views.end(), [screen_id](VkTextureView &v) { return v.id == screen_id; });

    if (found == _views.end())
    {
        return;
    }

    auto &view = *found;

    if (_rotation == CART_NO_ROTATE)
    {
        view.groupXDiv = 16;
        view.groupYDiv = 6;
    }
    else
    {
        view.groupXDiv = 6;
        view.groupYDiv = 16;
    }

    destroy_view_texture(view);
    destroy_view_compute(view);

    _swapChainRebuild = true;

    prepare_view_texture(view, VK_FORMAT_R8G8B8A8_UNORM);
    prepare_view_compute(view, sizeof(LynxScreenBuffer), _compute.screenViewShader, _compute.screenViewDescriptorSetLayout);

    LynxScreenBuffer *screenbuffer = (LynxScreenBuffer *)view.allocationInfo.pMappedData;
    screenbuffer->rotation = _rotation;
}

void VulkanRenderer::render_screen_view(int id, uint8_t *screen_buff, uint8_t *palette)
{
    if (screen_buff == nullptr || palette == nullptr)
    {
        return;
    }

    auto found = std::find_if(_views.begin(), _views.end(), [id](VkTextureView &v) { return v.id == id; });

    if (found == _views.end())
    {
        LOG(LOGLEVEL_ERROR) << "VkTextureView " << id << " not found.";
        return;
    }

    VkTextureView view = *found;
    LynxScreenBuffer *screen = (LynxScreenBuffer *)view.allocationInfo.pMappedData;

    memcpy(screen->buffer, screen_buff, SCREEN_HEIGHT * SCREEN_WIDTH / 2);
    memcpy(screen->palette, palette, 32);
}

ImTextureID VulkanRenderer::get_texture_ID(int viewId)
{
    auto view = std::find_if(_views.begin(), _views.end(), [viewId](const VkTextureView &v) { return v.id == viewId; });

    if (view == _views.end() || !view->ready || !view->texture.DS)
    {
        return nullptr;
    }

    return view->texture.DS;
}

void VulkanRenderer::prepare_view_texture(VkTextureView &view, VkFormat format)
{
    uint32_t w = HANDY_SCREEN_WIDTH;
    uint32_t h = HANDY_SCREEN_HEIGHT;

    if (_rotation != CART_NO_ROTATE)
    {
        w = HANDY_SCREEN_HEIGHT;
        h = HANDY_SCREEN_WIDTH;
    }

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &formatProperties);
    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    view.texture.width = w;
    view.texture.height = h;

    auto imageCreateInfo = vkinit::image_create_info(format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, {view.texture.width, view.texture.height, 1});

    std::vector<uint32_t> queueFamilyIndices;
    auto grQueue = _queueFamily;
    auto cpQueue = _compute.queueFamily;
    if (grQueue != cpQueue)
    {
        queueFamilyIndices = {grQueue, cpQueue};
        imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageCreateInfo.queueFamilyIndexCount = 2;
        imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocCreateInfo.priority = 1.0f;

    vmaCreateImage(_allocator, &imageCreateInfo, &allocCreateInfo, &view.texture.image, &view.allocation, nullptr);

    VkCommandBuffer layoutCmd = create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, _commandPool, true);

    view.texture.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    set_image_layout(layoutCmd, view.texture.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, view.texture.imageLayout);

    flush_command_buffer(layoutCmd, _queue, _commandPool, true);

    VkImageViewCreateInfo viewci = vkinit::imageview_create_info(format, view.texture.image, 0);
    viewci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VK_CHECK(vkCreateImageView(_device, &viewci, nullptr, &view.texture.view));

    VkSamplerCreateInfo sampler = vkinit::sampler_create_info(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.mipLodBias = 0.0f;
    sampler.anisotropyEnable = VK_FALSE;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    sampler.maxLod = VK_LOD_CLAMP_NONE;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    VK_CHECK(vkCreateSampler(_device, &sampler, nullptr, &view.texture.sampler));

    view.texture.descriptor.imageLayout = view.texture.imageLayout;
    view.texture.descriptor.imageView = view.texture.view;
    view.texture.descriptor.sampler = view.texture.sampler;
    view.texture.device = _device;

    view.texture.DS = ImGui_ImplVulkan_AddTexture(view.texture.sampler, view.texture.view, VK_IMAGE_LAYOUT_GENERAL);
}

bool VulkanRenderer::delete_view(int viewId)
{
    vkQueueWaitIdle(_compute.queue);

    auto view = std::find_if(_views.begin(), _views.end(), [viewId](const VkTextureView &b) { return b.id == viewId; });

    if (view == _views.end())
    {
        return false;
    }

    destroy_view_texture(*view);
    destroy_view_compute(*view);

    _swapChainRebuild = true;

    std::erase_if(_views, [viewId](const VkTextureView &v) { return v.id == viewId; });

    return true;
}

void VulkanRenderer::destroy_view_texture(VkTextureView &view)
{
    view.ready = false;
    if (view.texture.DS)
    {
        ImGui_ImplVulkan_RemoveTexture(view.texture.DS);
        view.texture.DS = nullptr;
    }
    vkDestroyImageView(_device, view.texture.view, nullptr);
    vmaDestroyImage(_allocator, view.texture.image, view.allocation);
}

int VulkanRenderer::add_screen_view(uint16_t baseAddress)
{
    vkQueueWaitIdle(_compute.queue);

    VktextureView v{};
    v.id = _viewId++;
    if (_rotation != CART_NO_ROTATE)
    {
        v.groupXDiv = 6;
        v.groupYDiv = 16;
    }

    prepare_view_texture(v, VK_FORMAT_R8G8B8A8_UNORM);
    prepare_view_compute(v, sizeof(LynxScreenBuffer), _compute.screenViewShader, _compute.screenViewDescriptorSetLayout);

    LynxScreenBuffer *screenbuffer = (LynxScreenBuffer *)v.allocationInfo.pMappedData;
    screenbuffer->rotation = _rotation;

    _views.push_back(v);

    return v.id;
}
