/*
    This file is part of Vulkan-Engine (VkFW), a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/

// With this file, you can easy include all core functionality ...

#pragma once

#include <engine/core/geometry.h>
#include <engine/core/material.h>
#include <engine/core/object3D.h>
#include <engine/core/renderer.h>
#include <engine/core/renderpass.h>
#include <engine/core/texture.h>
#include <engine/core/window.h>

#include <engine/core/materials/phong.h>
#include <engine/core/materials/physically_based.h>
#include <engine/core/materials/unlit.h>

#include <engine/core/scene/camera.h>
#include <engine/core/scene/light.h>
#include <engine/core/scene/mesh.h>
#include <engine/core/scene/scene.h>

#include <engine/core/renderpasses/composition_pass.h>
#include <engine/core/renderpasses/forward_pass.h>
#include <engine/core/renderpasses/fxaa_pass.h>
#include <engine/core/renderpasses/geometry_pass.h>
#include <engine/core/renderpasses/shadow_pass.h>
#include <engine/core/renderpasses/ssao_pass.h>
#include <engine/core/renderpasses/ssao_blur_pass.h>