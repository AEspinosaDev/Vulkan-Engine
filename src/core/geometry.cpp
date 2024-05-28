#include <engine/core/geometry.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Geometry::fill(std::vector<Vertex> vertexInfo)
{
  m_vertexData = vertexInfo;
  m_loaded = true;
  m_indexed = false;
}
void Geometry::fill(std::vector<Vertex> vertexInfo, std::vector<uint16_t> vertexIndex)
{
  if (vertexIndex.empty())
  {
    fill(vertexInfo);
    return;
  }
  m_vertexData = vertexInfo;
  m_vertexIndex = vertexIndex;
  m_loaded = true;
  m_indexed = true;
}

void Geometry::fill(Vec3 *pos, Vec3 *normal, Vec2 *uv, Vec3 *tangent, uint32_t vertNumber)
{
  for (size_t i = 0; i < vertNumber; i++)
  {
    m_vertexData.push_back({pos[i], normal[i], tangent[i], uv[i], Vec3(1.0)});
  }
}

void Geometry::draw(VkCommandBuffer &cmd, Geometry *const g)
{
  VkBuffer vertexBuffers[] = {g->m_vbo->buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

  if (g->m_indexed)
  {
    vkCmdBindIndexBuffer(cmd, g->m_ibo->buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, static_cast<uint32_t>(g->m_vertexIndex.size()), 1, 0, 0, 0);
  }
  else
  {
    vkCmdDraw(cmd, static_cast<uint32_t>(g->m_vertexData.size()), 1, 0, 0);
  }
}

void Geometry::upload_buffers(VkDevice &device, VmaAllocator &memory, VkQueue &gfxQueue, utils::UploadContext &uploadContext, Geometry *const g)
{

  // Should be executed only once if geometry data is not changed

  // Staging vertex buffer (CPU only)
  size_t vboSize = sizeof(g->m_vertexData[0]) * g->m_vertexData.size();
  Buffer vboStagingBuffer;
  vboStagingBuffer.init(memory, vboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
  vboStagingBuffer.upload_data(memory, g->m_vertexData.data(), vboSize);

  // GPU vertex buffer
  g->m_vbo->init(memory, vboSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

  uploadContext.immediate_submit(device, gfxQueue, [=](VkCommandBuffer cmd)
                                 {
				VkBufferCopy copy;
				copy.dstOffset = 0;
				copy.srcOffset = 0;
				copy.size = vboSize;
				vkCmdCopyBuffer(cmd, vboStagingBuffer.buffer, g->m_vbo->buffer, 1, &copy); });

 
  vboStagingBuffer.cleanup(memory);

  if (g->m_indexed)
  {
    // Staging index buffer (CPU only)
    size_t iboSize = sizeof(g->m_vertexIndex[0]) * g->m_vertexIndex.size();
    Buffer iboStagingBuffer;
    iboStagingBuffer.init(memory, iboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    iboStagingBuffer.upload_data(memory, g->m_vertexIndex.data(), iboSize);

    // GPU index buffer
    g->m_ibo->init(memory, iboSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    uploadContext.immediate_submit(device, gfxQueue, [=](VkCommandBuffer cmd)
                                   {

					VkBufferCopy index_copy;
					index_copy.dstOffset = 0;
					index_copy.srcOffset = 0;
					index_copy.size = iboSize;
					vkCmdCopyBuffer(cmd, iboStagingBuffer.buffer, g->m_ibo->buffer, 1, &index_copy); });
   
    iboStagingBuffer.cleanup(memory);
  }

  g->m_buffers_loaded = true;
}
void GeometryStats::compute_statistics(Geometry *g)
{
  maxCoords = {0.0f, 0.0f, 0.0f};
  minCoords = {INFINITY, INFINITY, INFINITY};

  for (const Vertex &v : g->get_vertex_data())
  {
    if (v.pos.x > maxCoords.x)
      maxCoords.x = v.pos.x;
    if (v.pos.y > maxCoords.y)
      maxCoords.y = v.pos.y;
    if (v.pos.z > maxCoords.z)
      maxCoords.z = v.pos.z;
    if (v.pos.x < minCoords.x)
      minCoords.x = v.pos.x;
    if (v.pos.y < minCoords.y)
      minCoords.y = v.pos.y;
    if (v.pos.z < minCoords.z)
      minCoords.z = v.pos.z;
  }

  center = (maxCoords + minCoords) * 0.5f;
}
VULKAN_ENGINE_NAMESPACE_END