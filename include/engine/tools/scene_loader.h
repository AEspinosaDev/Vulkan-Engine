/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include <engine/tools/loaders.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Tools {
/*Loads and save a scene from XML file*/
class SceneLoader
{
    bool            m_asyncLoad;
    Core::Transform load_transform(tinyxml2::XMLElement* obj);
    void            save_transform(const Core::Transform& transform, tinyxml2::XMLElement* parentElement);
    void            load_children(tinyxml2::XMLElement* element, Core::Object3D* const parent, std::string resourcesPath);
    void            save_children(tinyxml2::XMLElement* parentElement, Core::Object3D* const parent);

  public:
    SceneLoader(bool asyncLoading = true)
        : m_asyncLoad(asyncLoading) {
    }
    /*Loads a scene from an XML file*/
    void load_scene(Core::Scene* const scene, const std::string fileName);
    /*Saves a scene to an XML file*/
    void save_scene(Core::Scene* const scene, const std::string fileName);
};
} // namespace Tools
VULKAN_ENGINE_NAMESPACE_END
#endif