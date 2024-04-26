#include <engine/utilities/gui.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
bool GUIOverlay::m_initialized = false;

void GUIOverlay::init(VkInstance &instance, VkDevice &device, VkPhysicalDevice &gpu, VkQueue &graphicsQueue, VkRenderPass &renderPass,
                      VkFormat format, VkSampleCountFlagBits samples, GLFWwindow *window)
{
    if (GUIOverlay::m_initialized)
        return;

    // 1: create descriptor pool for IMGUI
    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &m_pool));

    // 2: initialize imgui library
    // this initializes the core structures of imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGui::SetCurrentContext()

    switch (m_colorProfile)
    {
    case BRIGHT:
        ImGui::StyleColorsLight();
        break;
    case DARK:
        ImGui::StyleColorsDark();
        break;
    case CLASSIC:
        ImGui::StyleColorsClassic();
        break;
    }

    // this initializes imgui for SDL
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = gpu;
    init_info.Device = device;
    init_info.Queue = graphicsQueue;
    init_info.DescriptorPool = m_pool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.RenderPass = renderPass;
    init_info.MSAASamples = samples;

    ImGui_ImplVulkan_Init(&init_info);

    GUIOverlay::m_initialized = true;
}

void GUIOverlay::render()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    for (auto p : m_panels)
    {
        p->render(m_extent);
        p->set_is_resized(m_resized);
    }
    ImGui::Render();

    m_resized = false;
}

void GUIOverlay::upload_draw_data(VkCommandBuffer &cmd)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void GUIOverlay::cleanup(VkDevice &device)
{
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(device, m_pool, nullptr);
}

VULKAN_ENGINE_NAMESPACE_END