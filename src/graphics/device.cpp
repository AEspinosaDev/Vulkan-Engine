#include <engine/graphics/device.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void Device::init( void*           windowHandle,
                   WindowingSystem windowingSystem,
                   Extent2D        surfaceExtent,
                   uint32_t        framesPerFlight,
                   ColorFormatType presentFormat,
                   SyncType        presentMode ) {

    // BOOT Vulkan ------>>>

    // Instance
    m_instance = Booter::create_instance( "xxx", "VulkanDevice", m_enableValidationLayers, m_validationLayers );
    if ( m_enableValidationLayers )
        m_debugMessenger = Booter::create_debug_messenger( m_instance );

    // Surface
    Extent2D actualExtent = m_swapchain.create_surface( m_instance, windowHandle, windowingSystem );

    // Get gpu
    m_gpu = Booter::pick_graphics_card_device( m_instance, m_swapchain.get_surface(), m_extensions );
    // Store properties
    vkGetPhysicalDeviceProperties( m_gpu, &m_properties );
    vkGetPhysicalDeviceFeatures( m_gpu, &m_features );
    vkGetPhysicalDeviceMemoryProperties( m_gpu, &m_memoryProperties );

    // Create logical device
    m_handle = Booter::create_logical_device( m_queues, m_gpu, m_features, m_swapchain.get_surface(), m_enableValidationLayers, m_validationLayers );

    // Setup VMA
    m_allocator = Booter::setup_memory( m_instance, m_handle, m_gpu );

    // Create swapchain
    m_swapchain.create( m_gpu, m_handle, actualExtent, surfaceExtent, framesPerFlight, Translator::get( presentFormat ), Translator::get( presentMode ) );

    create_upload_context();
    load_extensions( m_handle, m_instance );

    //------<<<
}

void Device::init_headless() {

    // BOOT Vulkan (HEADLESS MODE) ------>>>

    // Instance
    m_instance = Booter::create_instance( "xxx", "VulkanDevice", m_enableValidationLayers, m_validationLayers );
    if ( m_enableValidationLayers )
        m_debugMessenger = Booter::create_debug_messenger( m_instance );

    // Get gpu
    m_gpu = Booter::pick_graphics_card_device( m_instance, VK_NULL_HANDLE, m_extensions );
    vkGetPhysicalDeviceProperties( m_gpu, &m_properties );
    vkGetPhysicalDeviceFeatures( m_gpu, &m_features );
    vkGetPhysicalDeviceMemoryProperties( m_gpu, &m_memoryProperties );

    // Create logical device
    m_handle = Booter::create_logical_device( m_queues, m_gpu, m_features, VK_NULL_HANDLE, m_enableValidationLayers, m_validationLayers );

    // Setup VMA
    m_allocator = Booter::setup_memory( m_instance, m_handle, m_gpu );

    create_upload_context();
    load_extensions( m_handle, m_instance );

    //------<<<
}
void Device::update_swapchain( Extent2D surfaceExtent, uint32_t framesPerFlight, ColorFormatType presentFormat, SyncType presentMode ) {
    m_swapchain.create( m_gpu, m_handle, surfaceExtent, surfaceExtent, framesPerFlight, Translator::get( presentFormat ), Translator::get( presentMode ) );
}

void Device::cleanup() {

    m_uploadContext.cleanup();

    m_swapchain.cleanup();

    vmaDestroyAllocator( m_allocator );

    vkDestroyDevice( m_handle, nullptr );

    if ( m_enableValidationLayers )
    {
        Booter::destroy_debug_utils_messenger_EXT( m_instance, m_debugMessenger, nullptr );
    }

    m_swapchain.destroy_surface( m_instance );
    vkDestroyInstance( m_instance, nullptr );
}

Buffer Device::create_buffer_VMA( size_t allocSize, BufferUsageFlags usage, VmaMemoryUsage memoryUsage, uint32_t strideSize ) {

    Buffer buffer = {};

    VkBufferCreateInfo bufferInfo        = {};
    bufferInfo.sType                     = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext                     = nullptr;
    bufferInfo.size                      = allocSize;
    bufferInfo.usage                     = Translator::get( usage );
    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage                   = memoryUsage;

    VK_CHECK( vmaCreateBuffer( m_allocator, &bufferInfo, &vmaallocInfo, &buffer.handle, &buffer.allocation, nullptr ) );

    buffer.device     = m_handle;
    buffer.allocator  = m_allocator;
    buffer.size       = allocSize;
    buffer.strideSize = strideSize == 0 ? allocSize : strideSize;

    return buffer;
}
Buffer Device::create_buffer( size_t allocSize, BufferUsageFlags usage, MemoryPropertyFlags memoryProperties, uint32_t strideSize ) {
    Buffer buffer = {};

    VkBufferCreateInfo bufferCreateInfo {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size  = allocSize;
    bufferCreateInfo.usage = Translator::get( usage );
    VK_CHECK( vkCreateBuffer( m_handle, &bufferCreateInfo, nullptr, &buffer.handle ) );

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements( m_handle, buffer.handle, &memoryRequirements );
    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo {};
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext           = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize  = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = get_memory_type( memoryRequirements.memoryTypeBits, memoryProperties );

    VK_CHECK( vkAllocateMemory( m_handle, &memoryAllocateInfo, nullptr, &buffer.memory ) );
    VK_CHECK( vkBindBufferMemory( m_handle, buffer.handle, buffer.memory, 0 ) );

    buffer.device     = m_handle;
    buffer.size       = allocSize;
    buffer.strideSize = strideSize == 0 ? allocSize : strideSize;

    if ( ( memoryProperties & MEMORY_PROPERTY_HOST_COHERENT ) != 0 )
        buffer.coherence = true;

    return buffer;
}
Image Device::create_image( const Extent3D& extent, const ImageConfig& config, VmaMemoryUsage memoryUsage ) {
    Image img  = {};
    img.extent = extent;
    img.device = m_handle;
    img.memory = m_allocator;

    img.config = config;

    VmaAllocationCreateInfo img_allocinfo = {};
    img_allocinfo.usage                   = memoryUsage;

    // Check mip levels
    uint32_t maxMip    = static_cast<uint32_t>( std::floor( std::log2( std::max( extent.width, extent.height ) ) ) ) + 1;
    img.config.mipmaps = config.mipmaps <= maxMip ? config.mipmaps : maxMip;

    // Check layers
    img.config.layers = config.type == IMAGE_TYPE_CUBE ? CUBEMAP_FACES : config.layers;

    VkImageCreateInfo img_info = Init::image_create_info( Translator::get( img.config.format ),
                                                          Translator::get( img.config.usageFlags ),
                                                          extent,
                                                          img.config.mipmaps,
                                                          static_cast<VkSampleCountFlagBits>( config.samples ),
                                                          img.config.layers,
                                                          Translator::get( img.config.type ),
                                                          img.config.type == IMAGE_TYPE_CUBE ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0 );
    img_info.initialLayout     = Translator::get( config.initialLayout );

    VK_CHECK( vmaCreateImage( m_allocator, &img_info, &img_allocinfo, &img.handle, &img.allocation, nullptr ) );

    return img;
}
Texture Device::create_texture( const Extent3D& extent, ColorFormatType format, const TextureConfig& config ) {
    Texture tex;
    tex.config = config;

    ImageConfig imageConfig {};
    imageConfig.type = IMAGE_TYPE_2D;
    if ( config.type == TextureTypeFlagBits::TEXTURE_3D )
        imageConfig.type = IMAGE_TYPE_3D;
    if ( config.type == TextureTypeFlagBits::TEXTURE_1D || config.type == TextureTypeFlagBits::TEXTURE_1D_ARRAY )
        imageConfig.type = IMAGE_TYPE_1D;
    if ( config.type == TextureTypeFlagBits::TEXTURE_CUBE || config.type == TextureTypeFlagBits::TEXTURE_CUBE_ARRAY )
        imageConfig.type = IMAGE_TYPE_CUBE;

    imageConfig.mipmaps    = config.maxMipLevel;
    imageConfig.layers     = config.maxLayer;
    imageConfig.format     = format;
    imageConfig.usageFlags = IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST;

    if ( !tex.image )
        tex.image = new Image(); // Reserve memory
    else
    {
        if ( tex.image->handle )
        {
            tex.image->cleanup();
        }
    }
    *tex.image = create_image( extent, imageConfig );

    VkImageViewCreateInfo dview_info = Init::imageview_create_info( Translator::get( format ),
                                                                    tex.image->handle,
                                                                    Translator::get( config.type ),
                                                                    Utils::get_aspect( tex.image->config.format ),
                                                                    config.maxMipLevel,
                                                                    config.maxLayer,
                                                                    config.baseMipLevel );

    VK_CHECK( vkCreateImageView( m_handle, &dview_info, nullptr, &tex.viewHandle ) );

    if ( !config.sampled )
        return tex;

    VkSamplerCreateInfo samplerInfo = Init::sampler_create_info( Translator::get( config.filters ),
                                                                 Translator::get( config.mipmapMode ),
                                                                 config.baseMipLevel,
                                                                 config.maxMipLevel,
                                                                 config.maxAnysotropy == 0 ? false : true,
                                                                 config.maxAnysotropy,
                                                                 Translator::get( config.samplerAddressMode ) );
    samplerInfo.borderColor         = Translator::get( config.border );

    VK_CHECK( vkCreateSampler( m_handle, &samplerInfo, nullptr, &tex.samplerHandle ) );

    return tex;
}

Texture Device::create_texture( Image* img, TextureConfig config ) {
    Texture tex;
    tex.config = config;

    tex.image = img;

    VkImageViewCreateInfo dview_info = Init::imageview_create_info( Translator::get( img->config.format ),
                                                                    tex.image->handle,
                                                                    Translator::get( config.type ),
                                                                    Utils::get_aspect( img->config.format ),
                                                                    config.maxMipLevel,
                                                                    config.maxLayer,
                                                                    config.baseMipLevel );

    VK_CHECK( vkCreateImageView( m_handle, &dview_info, nullptr, &tex.viewHandle ) );

    if ( !config.sampled )
        return tex;

    VkSamplerCreateInfo samplerInfo = Init::sampler_create_info(
        Translator::get( config.filters ),
        Translator::get( config.mipmapMode ),
        config.baseMipLevel,
        config.maxMipLevel,
        config.maxAnysotropy == 0 ? false : true,
        config.maxAnysotropy > m_properties.limits.maxSamplerAnisotropy ? m_properties.limits.maxSamplerAnisotropy : config.maxAnysotropy,
        Translator::get( config.samplerAddressMode ) );
    samplerInfo.borderColor = Translator::get( config.border );

    VK_CHECK( vkCreateSampler( m_handle, &samplerInfo, nullptr, &tex.samplerHandle ) );

    return tex;
}
CommandPool Device::create_command_pool( QueueType QueueType, CommandPoolCreateFlags flags ) {
    CommandPool pool = {};
    pool.device      = m_handle;

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    switch ( QueueType )
    {
        case QueueType::GRAPHIC_QUEUE:
            poolInfo.queueFamilyIndex = Booter::find_queue_families( m_gpu, m_swapchain.get_surface() ).graphicsFamily.value();
            break;
        case QueueType::COMPUTE_QUEUE:
            poolInfo.queueFamilyIndex = Booter::find_queue_families( m_gpu, m_swapchain.get_surface() ).computeFamily.value();
            break;
        case QueueType::PRESENT_QUEUE:
            poolInfo.queueFamilyIndex = Booter::find_queue_families( m_gpu, m_swapchain.get_surface() ).presentFamily.value();
            break;
        default:
            break;
    }
    pool.queue     = m_queues[QueueType];
    poolInfo.flags = Translator::get( flags );

    if ( vkCreateCommandPool( m_handle, &poolInfo, nullptr, &pool.handle ) != VK_SUCCESS )
    {
        throw VKFW_Exception( "Failed to create command pool!" );
    }
    return pool;
}
CommandBuffer Device::create_command_buffer( CommandPool commandPool, CommandBufferLevel level ) {
    CommandBuffer cmd                        = {};
    cmd.device                               = m_handle;
    cmd.pool                                 = commandPool.handle;
    cmd.queue                                = commandPool.queue;
    VkCommandBufferAllocateInfo cmdAllocInfo = Init::command_buffer_allocate_info( commandPool.handle, 1, Translator::get( level ) );
    VK_CHECK( vkAllocateCommandBuffers( m_handle, &cmdAllocInfo, &cmd.handle ) );
    return cmd;
}

GraphicShaderPass Device::create_graphic_shader_pass( const std::string shaderFile, const std::vector<Graphics::DescriptorLayout>& descriptorLayouts, GraphicPipelineConfig& config, const RenderPass& renderPass, const std::vector<PushConstant>& pushConstants ) {
    GraphicShaderPass shaderPass;
    shaderPass.device = m_handle;

    shaderPass.compile_shader_stages( shaderFile );

    std::vector<VkDescriptorSetLayout> vkLayouts;
    vkLayouts.resize( descriptorLayouts.size() );
    for ( size_t i = 0; i < descriptorLayouts.size(); i++ )
    {
        vkLayouts[i] = descriptorLayouts[i].handle;
    }

    PipelineBuilder::build_pipeline_layout( shaderPass.pipelineLayout, m_handle, vkLayouts, pushConstants );

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    for ( auto& stage : shaderPass.shaderStages )
    {
        stages.push_back( Init::pipeline_shader_stage_create_info( stage.stage, stage.shaderModule ) );
    }
    PipelineBuilder::build_graphic_pipeline( shaderPass.pipeline, shaderPass.pipelineLayout, m_handle, renderPass.handle, { 0, 0 }, config, stages );

    return shaderPass;
}
ComputeShaderPass Device::create_compute_shader_pass( const std::string shaderFile, const std::vector<Graphics::DescriptorLayout>& descriptorLayouts, const std::vector<PushConstant>& pushConstants ) {
    ComputeShaderPass shaderPass;
    shaderPass.device = m_handle;

    shaderPass.compile_shader_stages( shaderFile );

    std::vector<VkDescriptorSetLayout> vkLayouts;
    vkLayouts.resize( descriptorLayouts.size() );
    for ( size_t i = 0; i < descriptorLayouts.size(); i++ )
    {
        vkLayouts[i] = descriptorLayouts[i].handle;
    }

    PipelineBuilder::build_pipeline_layout( shaderPass.pipelineLayout, m_handle, vkLayouts, pushConstants );

    PipelineBuilder::build_compute_pipeline(
        shaderPass.pipeline, shaderPass.pipelineLayout, m_handle, Init::pipeline_shader_stage_create_info( shaderPass.computeStage.stage, shaderPass.computeStage.shaderModule ) );

    return shaderPass;
}
DescriptorPool Device::create_descriptor_pool( uint32_t                       maxSets,
                                               uint32_t                       numUBO,
                                               uint32_t                       numUBODynamic,
                                               uint32_t                       numUBOStorage,
                                               uint32_t                       numImageCombined,
                                               uint32_t                       numSampler,
                                               uint32_t                       numSampledImage,
                                               uint32_t                       numStrgImage,
                                               uint32_t                       numUBTexel,
                                               uint32_t                       numStrgTexel,
                                               uint32_t                       numUBOStorageDynamic,
                                               uint32_t                       numIAttachment,
                                               VkDescriptorPoolCreateFlagBits flag ) {
    DescriptorPool pool = {};
    pool.device         = m_handle;

    std::vector<VkDescriptorPoolSize> sizes = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numUBO },
                                                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, numUBODynamic },
                                                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, numUBOStorage },
                                                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numImageCombined } };
    if ( numSampler > 0 )
        sizes.push_back( { VK_DESCRIPTOR_TYPE_SAMPLER, numSampler } );
    if ( numSampledImage > 0 )
        sizes.push_back( { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, numSampledImage } );
    if ( numStrgImage > 0 )
        sizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, numStrgImage } );
    if ( numUBTexel > 0 )
        sizes.push_back( { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, numUBTexel } );
    if ( numStrgTexel > 0 )
        sizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, numStrgTexel } );
    if ( numUBOStorageDynamic > 0 )
        sizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, numUBOStorageDynamic } );
    if ( numIAttachment > 0 )
        sizes.push_back( { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, numIAttachment } );

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags                      = flag;
    pool_info.maxSets                    = maxSets;
    pool_info.poolSizeCount              = (uint32_t)sizes.size();
    pool_info.pPoolSizes                 = sizes.data();

    VK_CHECK( vkCreateDescriptorPool( m_handle, &pool_info, nullptr, &pool.handle ) );
    return pool;
}
RenderPass Device::create_render_pass( const std::vector<RenderTargetInfo>& targets, const std::vector<SubPassDependency>& dependencies ) {
    RenderPass rp = {};
    // ATTACHMENT SETUP ----------------------------------
    rp.device = m_handle;

    std::vector<VkAttachmentDescription> attachmentsInfo;
    attachmentsInfo.resize( targets.size(), {} );

    std::vector<VkAttachmentReference> colorAttachmentRefs;
    bool                               hasDepthAttachment = false;
    VkAttachmentReference              depthAttachmentRef;
    bool                               hasResolveAttachment = false;
    std::vector<VkAttachmentReference> resolveAttachemtRefs;

    AttachmentType attachmentType;
    for ( size_t i = 0; i < attachmentsInfo.size(); i++ )
    {
        attachmentsInfo[i].format         = Translator::get( targets[i].format );
        attachmentsInfo[i].samples        = static_cast<VkSampleCountFlagBits>( targets[i].samples );
        attachmentsInfo[i].loadOp         = Translator::get( targets[i].load ? ATTACHMENT_LOAD_OP_LOAD : ATTACHMENT_LOAD_OP_CLEAR );
        attachmentsInfo[i].storeOp        = Translator::get( targets[i].store ? ATTACHMENT_STORE_OP_STORE : ATTACHMENT_STORE_OP_NONE );
        attachmentsInfo[i].stencilLoadOp  = Translator::get( targets[i].load ? ATTACHMENT_LOAD_OP_LOAD : ATTACHMENT_LOAD_OP_CLEAR );
        attachmentsInfo[i].stencilStoreOp = Translator::get( targets[i].store ? ATTACHMENT_STORE_OP_STORE : ATTACHMENT_STORE_OP_NONE );
        attachmentsInfo[i].initialLayout  = Translator::get( targets[i].initialLayout );
        attachmentsInfo[i].finalLayout    = Translator::get( targets[i].finalLayout );
        attachmentType                    = targets[i].resolve ? RESOLVE_ATTACHMENT : Utils::get_aspect( targets[i].format ) == ImageAspect::ASPECT_COLOR ? COLOR_ATTACHMENT
                                                                                                                                                          : DEPTH_ATTACHMENT;

        switch ( attachmentType )
        {
            case AttachmentType::COLOR_ATTACHMENT:
                colorAttachmentRefs.push_back( Init::attachment_reference( i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ) );
                break;
            case AttachmentType::DEPTH_ATTACHMENT:
                hasDepthAttachment = true;
                depthAttachmentRef = Init::attachment_reference( i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
                break;
            case AttachmentType::RESOLVE_ATTACHMENT:
                hasResolveAttachment = true;
                resolveAttachemtRefs.push_back( Init::attachment_reference( i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ) );
                break;
        }
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>( colorAttachmentRefs.size() );
    subpass.pColorAttachments    = colorAttachmentRefs.data();
    if ( hasDepthAttachment )
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    if ( hasResolveAttachment )
        subpass.pResolveAttachments = resolveAttachemtRefs.data();

    // SUBPASS DEPENDENCIES SETUP ----------------------------------

    std::vector<VkSubpassDependency> subpassDependencies;
    subpassDependencies.resize( dependencies.size(), {} );

    // Depdencies
    for ( size_t i = 0; i < subpassDependencies.size(); i++ )
    {
        subpassDependencies[i].srcSubpass      = dependencies[i].srcSubpass;
        subpassDependencies[i].dstSubpass      = dependencies[i].dstSubpass;
        subpassDependencies[i].srcStageMask    = Translator::get( dependencies[i].srcStageMask );
        subpassDependencies[i].dstStageMask    = Translator::get( dependencies[i].dstStageMask );
        subpassDependencies[i].srcAccessMask   = Translator::get( dependencies[i].srcAccessMask );
        subpassDependencies[i].dstAccessMask   = Translator::get( dependencies[i].dstAccessMask );
        subpassDependencies[i].dependencyFlags = Translator::get( dependencies[i].dependencyFlags );
    }

    // Creation
    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>( attachmentsInfo.size() );
    renderPassInfo.pAttachments    = attachmentsInfo.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>( subpassDependencies.size() );
    renderPassInfo.pDependencies   = subpassDependencies.data();

    if ( vkCreateRenderPass( m_handle, &renderPassInfo, nullptr, &rp.handle ) != VK_SUCCESS )
    {
        new VKFW_Exception( "failed to create renderpass!" );
    }
    rp.targetInfos  = targets;
    rp.dependencies = dependencies;
    return rp;
}

DescriptorLayout Device::create_descriptor_layout( const std::vector<LayoutBinding>& bindings, VkDescriptorSetLayoutCreateFlags flags, VkDescriptorBindingFlagsEXT extFlags ) {
    std::vector<VkDescriptorSetLayoutBinding> bindingHandles;
    bindingHandles.resize( bindings.size() );
    for ( size_t i = 0; i < bindings.size(); i++ )
    {
        bindingHandles[i] = bindings[i].handle;
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo = {};
    bindingFlagsInfo.sType                                          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    bindingFlagsInfo.bindingCount                                   = static_cast<uint32_t>( bindingHandles.size() );
    bindingFlagsInfo.pBindingFlags                                  = &extFlags;

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.bindingCount                    = static_cast<uint32_t>( bindingHandles.size() );
    setinfo.flags                           = flags;
    setinfo.pNext                           = extFlags != 0 ? &bindingFlagsInfo : nullptr;
    setinfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings                       = bindingHandles.data();

    DescriptorLayout layout {};
    VK_CHECK( vkCreateDescriptorSetLayout( m_handle, &setinfo, nullptr, &layout.handle ) );
    layout.device   = m_handle;
    layout.bindings = bindings;

    return layout;
}
Framebuffer Device::create_framebuffer( const RenderPass& renderpass, const std::vector<Image*>& attachments ) {
    assert( !attachments.empty() );

    Framebuffer fbo = {};
    fbo.device      = m_handle;
    fbo.layers      = attachments[0]->config.layers;

    // Populate Image Attachments
    const auto NUM_ATTACHMENTS = attachments.size();
    fbo.attachmentViews.resize( NUM_ATTACHMENTS );

    for ( size_t i = 0; i < NUM_ATTACHMENTS; i++ )
    {
        auto                  img = attachments[i];
        VkImageViewCreateInfo dview_info =
            Init::imageview_create_info( Translator::get( img->config.format ),
                                         img->handle,
                                         fbo.layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, // Only 2D supported
                                         Utils::get_aspect( img->config.format ),
                                         1, // One mip supported
                                         img->config.layers );

        VK_CHECK( vkCreateImageView( m_handle, &dview_info, nullptr, &fbo.attachmentViews[i] ) );
    }

    VkFramebufferCreateInfo fbInfo = Init::framebuffer_create_info( renderpass.handle, { attachments[0]->extent.width, attachments[0]->extent.height } );
    fbInfo.pAttachments            = fbo.attachmentViews.data();
    fbInfo.attachmentCount         = (uint32_t)fbo.attachmentViews.size();
    fbInfo.layers                  = fbo.layers;

    if ( vkCreateFramebuffer( m_handle, &fbInfo, nullptr, &fbo.handle ) != VK_SUCCESS )
    {
        throw VKFW_Exception( "failed to create framebuffer!" );
    }
    return fbo;
}
Semaphore Device::create_semaphore() {
    Semaphore semaphore                       = {};
    semaphore.device                          = m_handle;
    VkSemaphoreCreateInfo semaphoreCreateInfo = Init::semaphore_create_info();
    VK_CHECK( vkCreateSemaphore( m_handle, &semaphoreCreateInfo, nullptr, &semaphore.handle ) );
    return semaphore;
}
Fence Device::create_fence( bool signaled ) {
    Fence fence                       = {};
    fence.device                      = m_handle;
    VkFenceCreateInfo fenceCreateInfo = Init::fence_create_info( signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0U );
    VK_CHECK( vkCreateFence( m_handle, &fenceCreateInfo, nullptr, &fence.handle ) );
    return fence;
}

RenderResult Device::aquire_present_image( Semaphore& waitSemahpore, uint32_t& imageIndex ) {

    VkResult result = vkAcquireNextImageKHR( m_handle, m_swapchain.get_handle(), UINT64_MAX, waitSemahpore.handle, VK_NULL_HANDLE, &imageIndex );
    return static_cast<RenderResult>( result );
}

RenderResult Device::present_image( Semaphore& signalSemaphore, uint32_t imageIndex ) {

    VkSemaphore      signalSemaphores[] = { signalSemaphore.handle };
    VkPresentInfoKHR presentInfo        = Init::present_info();
    presentInfo.sType                   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount      = 1;
    presentInfo.pWaitSemaphores         = signalSemaphores;
    VkSwapchainKHR swapChains[]         = { m_swapchain.get_handle() };
    presentInfo.swapchainCount          = 1;
    presentInfo.pSwapchains             = swapChains;
    presentInfo.pImageIndices           = &imageIndex;

    VkResult result = vkQueuePresentKHR( m_queues[QueueType::PRESENT_QUEUE], &presentInfo );
    return static_cast<RenderResult>( result );
}

void Device::upload_vertex_arrays( VertexArrays& vao,
                                   size_t        vboSize,
                                   const void*   vboData,
                                   size_t        iboSize,
                                   const void*   iboData,
                                   size_t        voxelSize,
                                   const void*   voxelData ) {
    PROFILING_EVENT()
    // Should be executed only once if geometry data is not changed

    Buffer vboStagingBuffer = create_buffer_VMA( vboSize, BUFFER_USAGE_TRANSFER_SRC, VMA_MEMORY_USAGE_CPU_ONLY );
    vboStagingBuffer.upload_data( vboData, vboSize );

    // GPU vertex buffer
    vao.vbo = create_buffer_VMA(
        vboSize,
        BUFFER_USAGE_VERTEX_BUFFER | BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_SHADER_DEVICE_ADDRESS | BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY,
        VMA_MEMORY_USAGE_GPU_ONLY );

    m_uploadContext.immediate_submit( [&]( CommandBuffer cmd ) { cmd.copy_buffer( vboStagingBuffer, vao.vbo, vboSize ); } );

    vboStagingBuffer.cleanup();

    if ( vao.indexCount > 0 )
    {
        // Staging index buffer (CPU only)
        Buffer iboStagingBuffer = create_buffer_VMA( iboSize, BUFFER_USAGE_TRANSFER_SRC, VMA_MEMORY_USAGE_CPU_ONLY );
        iboStagingBuffer.upload_data( iboData, iboSize );

        // GPU index buffer
        vao.ibo = create_buffer_VMA( iboSize,
                                     BUFFER_USAGE_INDEX_BUFFER | BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_SHADER_DEVICE_ADDRESS |
                                         BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY,
                                     VMA_MEMORY_USAGE_GPU_ONLY );

        m_uploadContext.immediate_submit( [&]( CommandBuffer cmd ) { cmd.copy_buffer( iboStagingBuffer, vao.ibo, iboSize ); } );

        iboStagingBuffer.cleanup();
    }
    if ( vao.voxelCount > 0 )
    {
        // Staging Voxel buffer (CPU only)
        Buffer voxelStagingBuffer = create_buffer_VMA( voxelSize, BUFFER_USAGE_TRANSFER_SRC, VMA_MEMORY_USAGE_CPU_ONLY );
        voxelStagingBuffer.upload_data( voxelData, voxelSize );

        // GPU Voxel buffer
        vao.voxelBuffer =
            create_buffer_VMA( voxelSize,
                               BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_SHADER_DEVICE_ADDRESS | BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY,
                               VMA_MEMORY_USAGE_GPU_ONLY );

        m_uploadContext.immediate_submit( [&]( CommandBuffer cmd ) { cmd.copy_buffer( voxelStagingBuffer, vao.voxelBuffer, voxelSize ); } );

        voxelStagingBuffer.cleanup();
    }

    vao.loadedOnGPU = true;
}
void Device::upload_texture_image( Texture& tex, const void* imgCache, size_t bytesPerPixel ) {
    PROFILING_EVENT()

    VkDeviceSize imageSize = tex.get_extent().width * tex.get_extent().height * tex.get_extent().depth * bytesPerPixel;

    Buffer stagingBuffer = create_buffer_VMA( imageSize, BUFFER_USAGE_TRANSFER_SRC, VMA_MEMORY_USAGE_CPU_ONLY );
    stagingBuffer.upload_data( imgCache, static_cast<size_t>( imageSize ) );

    m_uploadContext.immediate_submit( [&]( CommandBuffer cmd ) { cmd.copy_buffer_to_image( *tex.image, stagingBuffer ); } );

    stagingBuffer.cleanup();

    // GENERATE MIPMAPS
    if ( tex.config.maxMipLevel > 1 )
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties( m_gpu, Translator::get( tex.image->config.format ), &formatProperties );
        if ( !( formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT ) )
        {
            throw std::runtime_error( "texture image format does not support linear blitting!" );
        }

        m_uploadContext.immediate_submit( [&]( CommandBuffer cmd ) { cmd.generate_mipmaps( *tex.image ); } );
    }

    // CREATE SAMPLER
    // samplerConfig.mipmapMode    = MipmapMode::MIPMAP_LINEAR;
    // samplerConfig.maxAnysotropy = m_properties.limits.maxSamplerAnisotropy;
    // img.create_sampler(samplerConfig);

    if ( ImGui::GetCurrentContext() )
        tex.create_GUI_handle();

    tex.image->empty = false;
}
void Device::upload_BLAS( BLAS& accel, VAO& vao ) {
    if ( !vao.loadedOnGPU )
        return;
    if ( accel.handle && !accel.dynamic )
        accel.cleanup();

    // GEOMETRY -----------------------------------------------------------
    VkDeviceOrHostAddressConstKHR      vertexBufferDeviceAddress     = {};
    VkDeviceOrHostAddressConstKHR      indexBufferDeviceAddress      = {};
    VkDeviceOrHostAddressConstKHR      voxelBufferDeviceAddress      = {};
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry = Init::acceleration_structure_geometry();

    if ( accel.topology == AccelGeometryType::TRIANGLES )
    {
        vertexBufferDeviceAddress.deviceAddress = vao.vbo.get_device_address();
        if ( vao.indexCount > 0 )
            indexBufferDeviceAddress.deviceAddress = vao.ibo.get_device_address();

        accelerationStructureGeometry.flags                           = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometryType                    = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelerationStructureGeometry.geometry.triangles.sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        accelerationStructureGeometry.geometry.triangles.vertexData   = vertexBufferDeviceAddress;
        accelerationStructureGeometry.geometry.triangles.maxVertex    = vao.vertexCount - 1;
        accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof( Vertex );

        if ( vao.indexCount > 0 )
        {
            accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
            accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
        }
        accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
        accelerationStructureGeometry.geometry.triangles.transformData.hostAddress   = nullptr;
    }
    if ( accel.topology == AccelGeometryType::AABBs )
    {
        voxelBufferDeviceAddress.deviceAddress = vao.voxelBuffer.get_device_address();

        accelerationStructureGeometry.flags                = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometryType         = VK_GEOMETRY_TYPE_AABBS_KHR;
        accelerationStructureGeometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;

        accelerationStructureGeometry.geometry.aabbs.data.deviceAddress = vao.voxelBuffer.get_device_address();
        accelerationStructureGeometry.geometry.aabbs.stride             = sizeof( Voxel ); // Stride between AABBs
    }

    // SIZE INFO -----------------------------------------------------------
    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = Init::acceleration_structure_build_geometry_info();
    accelerationStructureBuildGeometryInfo.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationStructureBuildGeometryInfo.flags =
        accel.dynamic ? VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR
                      : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationStructureBuildGeometryInfo.geometryCount = 1;
    accelerationStructureBuildGeometryInfo.pGeometries   = &accelerationStructureGeometry;

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo = Init::acceleration_structure_build_sizes_info();

    const uint32_t numPrimitives = accel.topology == AccelGeometryType::TRIANGLES ? vao.indexCount / 3 : vao.voxelCount;
    vkGetAccelerationStructureBuildSizes( m_handle,
                                          VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                          &accelerationStructureBuildGeometryInfo,
                                          &numPrimitives,
                                          &accelerationStructureBuildSizesInfo );

    // CREATE ACCELERATION BUFFER -----------------------------------------------------------
    if ( !accel.handle )
        accel.buffer = create_buffer( accelerationStructureBuildSizesInfo.accelerationStructureSize,
                                      BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE | BUFFER_USAGE_SHADER_DEVICE_ADDRESS,
                                      MEMORY_PROPERTY_DEVICE_LOCAL );

    // CREATE ACCELERATION STRUCTURE -----------------------------------------------------------
    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo {};
    accelerationStructureCreateInfo.sType  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreateInfo.buffer = accel.buffer.handle;
    accelerationStructureCreateInfo.size   = accelerationStructureBuildSizesInfo.accelerationStructureSize;
    accelerationStructureCreateInfo.type   = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    if ( vkCreateAccelerationStructure( m_handle, &accelerationStructureCreateInfo, nullptr, &accel.handle ) != VK_SUCCESS )
    {
        throw VKFW_Exception( "Failed to create BLAS!" );
    }

    // Create a small scratch buffer used during build of the bottom level acceleration structure
    Buffer scratchBuffer = create_buffer( accel.buffer.size, BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_SHADER_DEVICE_ADDRESS, MEMORY_PROPERTY_DEVICE_LOCAL );

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo {};
    accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    accelerationBuildGeometryInfo.type  = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationBuildGeometryInfo.flags = accel.dynamic
                                              ? VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR
                                              : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationBuildGeometryInfo.mode =
        accel.dynamic && accel.handle ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure  = accel.handle;
    accelerationBuildGeometryInfo.geometryCount             = 1;
    accelerationBuildGeometryInfo.pGeometries               = &accelerationStructureGeometry;
    accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.get_device_address();

    VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo {};
    accelerationStructureBuildRangeInfo.primitiveCount                                          = numPrimitives;
    accelerationStructureBuildRangeInfo.primitiveOffset                                         = 0;
    accelerationStructureBuildRangeInfo.firstVertex                                             = 0;
    accelerationStructureBuildRangeInfo.transformOffset                                         = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

    m_uploadContext.immediate_submit( [&]( CommandBuffer cmd ) {
        vkCmdBuildAccelerationStructures( cmd.handle, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data() );
    } );

    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo {};
    accelerationDeviceAddressInfo.sType                 = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    accelerationDeviceAddressInfo.accelerationStructure = accel.handle;
    accel.deviceAdress                                  = vkGetAccelerationStructureDeviceAddress( m_handle, &accelerationDeviceAddressInfo );

    scratchBuffer.cleanup();

    accel.device = m_handle;
}

void Device::upload_TLAS( TLAS& accel, std::vector<BLASInstance>& BLASinstances ) {
    if ( accel.handle && !accel.dynamic )
        accel.cleanup();

    // Set up instance data for each BLAS
    accel.instances = BLASinstances.size();

    std::vector<VkAccelerationStructureInstanceKHR> instances;
    instances.resize( BLASinstances.size(), {} );

    for ( size_t i = 0; i < BLASinstances.size(); ++i )
    {
        VkTransformMatrixKHR transformMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
        for ( int row = 0; row < 3; ++row )
            for ( int col = 0; col < 4; ++col )
                transformMatrix.matrix[row][col] = BLASinstances[i].transform[col][row]; // Column-major to Row-major

        instances[i].transform                              = transformMatrix;
        instances[i].instanceCustomIndex                    = i;
        instances[i].mask                                   = 0xFF;
        instances[i].instanceShaderBindingTableRecordOffset = 0;
        instances[i].flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instances[i].accelerationStructureReference         = BLASinstances[i].accel.deviceAdress;
    }

    // Create a buffer for the instances -----------------------------------------------------------
    Buffer instanceBuffer = create_buffer( sizeof( VkAccelerationStructureInstanceKHR ) * instances.size(),
                                           BUFFER_USAGE_SHADER_DEVICE_ADDRESS | BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY,
                                           MEMORY_PROPERTY_HOST_VISIBLE | MEMORY_PROPERTY_HOST_COHERENT );
    instanceBuffer.upload_data( instances.data(), sizeof( VkAccelerationStructureInstanceKHR ) * instances.size() );

    VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress {};
    instanceDataDeviceAddress.deviceAddress = instanceBuffer.get_device_address();

    VkAccelerationStructureGeometryKHR accelerationStructureGeometry = Init::acceleration_structure_geometry();
    accelerationStructureGeometry.geometryType                       = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    accelerationStructureGeometry.flags                              = VK_GEOMETRY_OPAQUE_BIT_KHR;
    accelerationStructureGeometry.geometry.instances.sType           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
    accelerationStructureGeometry.geometry.instances.data            = instanceDataDeviceAddress;

    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = Init::acceleration_structure_build_geometry_info();
    accelerationStructureBuildGeometryInfo.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    accelerationStructureBuildGeometryInfo.flags =
        accel.dynamic ? VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR
                      : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationStructureBuildGeometryInfo.geometryCount = 1;
    accelerationStructureBuildGeometryInfo.pGeometries   = &accelerationStructureGeometry;

    uint32_t primitiveCount = static_cast<uint32_t>( instances.size() );

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo = Init::acceleration_structure_build_sizes_info();
    vkGetAccelerationStructureBuildSizes( m_handle,
                                          VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                          &accelerationStructureBuildGeometryInfo,
                                          &primitiveCount,
                                          &accelerationStructureBuildSizesInfo );

    // CREATE ACCELERATION BUFFER
    if ( !accel.handle )
        accel.buffer = create_buffer( accelerationStructureBuildSizesInfo.accelerationStructureSize,
                                      BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE | BUFFER_USAGE_SHADER_DEVICE_ADDRESS,
                                      MEMORY_PROPERTY_DEVICE_LOCAL );

    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo {};
    accelerationStructureCreateInfo.sType  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreateInfo.buffer = accel.buffer.handle;
    accelerationStructureCreateInfo.size   = accelerationStructureBuildSizesInfo.accelerationStructureSize;
    accelerationStructureCreateInfo.type   = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

    if ( vkCreateAccelerationStructure( m_handle, &accelerationStructureCreateInfo, nullptr, &accel.handle ) != VK_SUCCESS )
    {
        throw VKFW_Exception( "Failed to create TLAS!" );
    };

    // Create a small scratch buffer used during build of the bottom level acceleration structure
    Buffer scratchBuffer = create_buffer(
        accelerationStructureBuildSizesInfo.buildScratchSize, BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_SHADER_DEVICE_ADDRESS, MEMORY_PROPERTY_DEVICE_LOCAL );

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo = Init::acceleration_structure_build_geometry_info();
    accelerationBuildGeometryInfo.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    accelerationBuildGeometryInfo.flags                                       = accel.dynamic
                                                                                    ? VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR
                                                                                    : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationBuildGeometryInfo.mode =
        accel.dynamic && accel.handle ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure  = accel.handle;
    accelerationBuildGeometryInfo.geometryCount             = 1;
    accelerationBuildGeometryInfo.pGeometries               = &accelerationStructureGeometry;
    accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.get_device_address();

    VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo {};
    accelerationStructureBuildRangeInfo.primitiveCount                                          = primitiveCount;
    accelerationStructureBuildRangeInfo.primitiveOffset                                         = 0;
    accelerationStructureBuildRangeInfo.firstVertex                                             = 0;
    accelerationStructureBuildRangeInfo.transformOffset                                         = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

    m_uploadContext.immediate_submit( [&]( CommandBuffer cmd ) {
        vkCmdBuildAccelerationStructures( cmd.handle, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data() );
    } );

    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo {};
    accelerationDeviceAddressInfo.sType                 = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    accelerationDeviceAddressInfo.accelerationStructure = accel.handle;
    accel.deviceAdress                                  = vkGetAccelerationStructureDeviceAddress( m_handle, &accelerationDeviceAddressInfo );

    scratchBuffer.cleanup();
    instanceBuffer.cleanup();

    accel.device = m_handle;
}
void Device::download_texture_image( Texture& tex, void*& imgCache, size_t& size, size_t& channels ) {
    channels                      = Utils::get_channel_count( tex.image->config.format );
    const uint32_t SIZE_PER_PIXEL = Utils::get_pixel_size_in_bytes( tex.image->config.format );
    const uint32_t SIZE_IN_BYTES  = tex.image->extent.width * tex.image->extent.height * tex.image->extent.depth * SIZE_PER_PIXEL;
    size                          = SIZE_IN_BYTES;

    imgCache         = malloc( SIZE_IN_BYTES );
    Buffer cpuBuffer = create_buffer( SIZE_IN_BYTES, BUFFER_USAGE_TRANSFER_DST, MEMORY_PROPERTY_HOST_VISIBLE, MEMORY_PROPERTY_HOST_COHERENT );

    m_uploadContext.immediate_submit( [&]( CommandBuffer cmd ) { cmd.copy_image_to_buffer( *tex.image, cpuBuffer ); } );

    cpuBuffer.copy_to( imgCache );

    cpuBuffer.cleanup();
}
void Device::wait_idle() {
    VK_CHECK( vkDeviceWaitIdle( m_handle ) );
}

void Device::wait_queue_idle( QueueType queueType ) {
    VK_CHECK( vkQueueWaitIdle( m_queues[queueType] ) );
}
void Device::init_imgui( void* windowHandle, WindowingSystem windowingSystem, RenderPass renderPass, uint16_t samples ) {

    m_guiPool =
        create_descriptor_pool( 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan( static_cast<GLFWwindow*>( windowHandle ), true );

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = m_instance;
    init_info.PhysicalDevice            = m_gpu;
    init_info.Device                    = m_handle;
    init_info.Queue                     = m_queues[QueueType::GRAPHIC_QUEUE];
    init_info.DescriptorPool            = m_guiPool.handle;
    init_info.MinImageCount             = 3;
    init_info.ImageCount                = 3;
    init_info.RenderPass                = renderPass.handle;
    init_info.MSAASamples               = static_cast<VkSampleCountFlagBits>( samples );

    ImGui_ImplVulkan_Init( &init_info );
}
void Device::destroy_imgui() {
    ImGui_ImplVulkan_Shutdown();
    m_guiPool.cleanup();
} // namespace Graphics

uint32_t Device::get_memory_type( uint32_t typeBits, MemoryPropertyFlags properties, uint32_t* memTypeFound ) {
    VkMemoryPropertyFlags vkproperties = Translator::get( properties );
    for ( uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++ )
    {
        if ( ( typeBits & 1 ) == 1 )
        {
            if ( ( m_memoryProperties.memoryTypes[i].propertyFlags & vkproperties ) == vkproperties )
            {
                if ( memTypeFound )
                {
                    *memTypeFound = true;
                }
                return i;
            }
        }
        typeBits >>= 1;
    }

    if ( memTypeFound )
    {
        *memTypeFound = false;
        return 0;
    } else
    {
        throw std::runtime_error( "Could not find a matching memory type" );
    }
}
size_t Device::pad_uniform_buffer_size( size_t originalSize ) {
    size_t minUboAlignment = m_properties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize     = originalSize;
    if ( minUboAlignment > 0 )
    {
        alignedSize = ( alignedSize + minUboAlignment - 1 ) & ~( minUboAlignment - 1 );
    }
    return alignedSize;
}

void Device::create_upload_context() {
    m_uploadContext.uploadFence   = create_fence( false );
    m_uploadContext.commandPool   = create_command_pool( QueueType::GRAPHIC_QUEUE );
    m_uploadContext.commandBuffer = create_command_buffer( m_uploadContext.commandPool );
}

void Device::UploadContext::immediate_submit( std::function<void( CommandBuffer cmd )>&& function ) {

    commandBuffer.begin();

    function( commandBuffer );

    commandBuffer.end();
    commandBuffer.submit( uploadFence );

    uploadFence.wait( 9999999999 );
    uploadFence.reset();

    commandPool.reset();
}

void Device::UploadContext::cleanup() {
    uploadFence.cleanup();
    commandPool.cleanup();
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
