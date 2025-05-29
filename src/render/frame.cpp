#include <engine/render/frame.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

void Frame::init( const std::shared_ptr<Graphics::Device>& device, uint32_t index ) {
    m_index            = index;
    m_commandPool      = device->create_command_pool( QueueType::GRAPHIC_QUEUE, COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER );
    m_commandBuffer    = device->create_command_buffer( m_commandPool );
    m_renderFence      = device->create_fence();
    m_renderSemaphore  = device->create_semaphore();
    m_presentSemaphore = device->create_semaphore();

    m_descriptorPool = device->create_descriptor_pool( 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT );

    m_device = device;
}

void Frame::wait() {
    m_renderFence.wait();
}
void Frame::start() {
    m_renderFence.reset();
    m_commandBuffer.reset();
    m_commandBuffer.begin();
}
void Frame::end() {
    m_commandBuffer.end();
}
void Frame::submit() {
    m_commandBuffer.submit( m_renderFence, { m_presentSemaphore }, { m_renderSemaphore } );
}

void Frame::cleanup() {

    m_descriptorPool.cleanup();
    m_commandPool.cleanup();
    m_renderFence.cleanup();
    m_renderSemaphore.cleanup();
    m_presentSemaphore.cleanup();
}

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END