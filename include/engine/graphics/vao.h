/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VAO_H
#define VAO_H

#include <engine/graphics/buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
/*
Geometric Render Data
*/
struct VertexArrays {
    bool     loadedOnGPU = false;
    Buffer   vbo         = {};
    uint32_t vertexCount = 0;
    Buffer   ibo         = {};
    uint32_t indexCount  = 0;
};
typedef VertexArrays VAO;

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif