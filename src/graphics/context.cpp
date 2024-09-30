#include <engine/graphics/context.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Context::init(GLFWwindow *windowHandle, VkExtent2D surfaceExtent, uint32_t framesPerFlight, VkFormat presentFormat, VkPresentModeKHR presentMode)
{

    // BOOT Vulkan ------>>>

    boot::VKBooter booter(enableValidationLayers);

    instance = booter.boot_vulkan();
    debugMessenger = booter.create_debug_messenger(instance);

    VK_CHECK(glfwCreateWindowSurface(instance, windowHandle, nullptr, &surface));

    // Get gpu
    gpu = booter.pick_graphics_card_device(instance, surface);

    // Create logical device
    device = booter.create_logical_device(
        graphicsQueue,
        presentQueue,
        gpu,
        utils::get_gpu_features(gpu),
        surface);

    // Setup VMA
    memory = booter.setup_memory(instance, device, gpu);

    uploadContext.init(device, gpu, surface);

    // Create swapchain
    swapchain.create(gpu, device, surface, windowHandle, surfaceExtent, framesPerFlight, presentFormat, presentMode);

    //------<<<
}

void Context::recreate_swapchain(GLFWwindow *windowHandle, VkExtent2D surfaceExtent, uint32_t framesPerFlight, VkFormat presentFormat, VkPresentModeKHR presentMode)
{
    swapchain.cleanup(device, memory);
    swapchain.create(gpu, device, surface, windowHandle, surfaceExtent, framesPerFlight, presentFormat, presentMode);
}

void Context::cleanup()
{
    swapchain.cleanup(device, memory);

    vmaDestroyAllocator(memory);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers)
    {
        utils::destroy_debug_utils_messenger_EXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

uint32_t Context::aquire_present_image(Frame &currentFrame)
{
    VK_CHECK(vkWaitForFences(device, 1, &currentFrame.renderFence, VK_TRUE, UINT64_MAX));
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain.get_handle(), UINT64_MAX, currentFrame.presentSemaphore, VK_NULL_HANDLE, &imageIndex);

    // if (result == VK_ERROR_OUT_OF_DATE_KHR)
    //     update_renderpasses();
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) throw VKException("failed to acquire swap chain image!");

    return imageIndex;
}

void Context::begin_command_buffer(Frame &currentFrame)
{
    VK_CHECK(vkResetFences(device, 1, &currentFrame.renderFence));
    VK_CHECK(vkResetCommandBuffer(currentFrame.commandBuffer, 0));

    VkCommandBufferBeginInfo beginInfo = init::command_buffer_begin_info();

    if (vkBeginCommandBuffer(currentFrame.commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw VKException("failed to begin recording command buffer!");
    }
}

void Context::end_command_buffer(Frame &currentFrame)
{
    if (vkEndCommandBuffer(currentFrame.commandBuffer) != VK_SUCCESS)
    {
        throw VKException("failed to record command buffer!");
    }
}

VkResult Context::present_image(Frame &currentFrame, uint32_t imageIndex)
{
    VkSubmitInfo submitInfo = init::submit_info(&currentFrame.commandBuffer);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {currentFrame.presentSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    VkSemaphore signalSemaphores[] = {currentFrame.renderSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, currentFrame.renderFence) != VK_SUCCESS)
    {
        throw VKException("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = init::present_info();
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapchain.get_handle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(presentQueue, &presentInfo);
}

void Context::wait_for_device()
{
    VK_CHECK(vkDeviceWaitIdle(device));
}

VULKAN_ENGINE_NAMESPACE_END