/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef UNIFORMS_H
#define UNIFORMS_H

#include <engine/graphics/buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

struct CameraUniforms
{
    Mat4 view;
    Mat4 proj;
    Mat4 viewProj;
    Vec4 position;
    Vec2 screenExtent;
};

struct LightUniforms
{
    Vec4 position = {0.0f, 0.0f, 0.0f, 0.0f}; // w for type
    Vec4 color = {0.0f, 0.0f, 0.0f, 0.0f};
    Vec4 dataSlot1 = {0.0f, 0.0f, 0.0f, 0.0f};
    Mat4 viewProj;
    Vec4 dataSlot2 = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct SceneUniforms
{
    Vec4 fogColorAndSSAO;     // w is for enabling SSAO
    Vec4 fogParams;    // x for near, y for far, z for intensity, w enable.
    Vec4 ambientColor; // w intensity
    LightUniforms lightUniforms[VK_MAX_LIGHTS];
    int numLights;
    int SSAOtype;
    int emphasizeAO;
};

struct ObjectUniforms
{
    Mat4 model;
    Vec4 otherParams1; // x is affected by fog, y is receive shadows, z cast shadows
    Vec4 otherParams2; // x is selected
};

struct MaterialUniforms
{
    Vec4 dataSlot1;
    Vec4 dataSlot2;
    Vec4 dataSlot3;
    Vec4 dataSlot4;
    Vec4 dataSlot5;
    Vec4 dataSlot6;
};

VULKAN_ENGINE_NAMESPACE_END

#endif