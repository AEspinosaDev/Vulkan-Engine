/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#pragma once
#include <engine/core/textures/texture_template.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

// g = gamma corrected (SRGB)
// u = unorm
// f = float (HDR)

namespace Core {

// 2D textures
using Texture2D  = TextureTemplate<DynamicRange::Uint, TextureType::TEXTURE_2D>;                      // Char 2D texture in Linear Space
using Texture2Du = TextureTemplate<DynamicRange::Unorm, TextureType::TEXTURE_2D>;                     // Normalized Char 2D texture in Linear Space
using Texture2Dg = TextureTemplate<DynamicRange::Uint, TextureType::TEXTURE_2D, ColorEncoding::SRGB>; // Char 2D texture in Gamma Spce (SRGB)
using Texture2Df = TextureTemplate<DynamicRange::Float, TextureType::TEXTURE_2D>;                     // Float 2D texture in Linear Space
// 3D textures
using Texture3D  = TextureTemplate<DynamicRange::Uint, TextureType::TEXTURE_3D>;                      // Char 3D texture in Linear Space
using Texture3Du = TextureTemplate<DynamicRange::Unorm, TextureType::TEXTURE_3D>;                     // Normalized Char 3D texture in Linear Space
using Texture3Dg = TextureTemplate<DynamicRange::Uint, TextureType::TEXTURE_3D, ColorEncoding::SRGB>; // Char 3D texture in Gamma Spce (SRGB)
using Texture3Df = TextureTemplate<DynamicRange::Float, TextureType::TEXTURE_3D>;                     // Float 3D texture in Linear Space
// Cubemaps
using Cubemap  = TextureTemplate<DynamicRange::Uint, TextureType::TEXTURE_CUBE>;
using Cubemapu = TextureTemplate<DynamicRange::Unorm, TextureType::TEXTURE_CUBE>;
using Cubemapg = TextureTemplate<DynamicRange::Uint, TextureType::TEXTURE_CUBE, ColorEncoding::SRGB>;
using Cubemapf = TextureTemplate<DynamicRange::Float, TextureType::TEXTURE_CUBE>;

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END
