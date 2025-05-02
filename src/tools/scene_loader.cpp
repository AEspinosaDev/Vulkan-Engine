#include <engine/tools/scene_loader.h>

VKFW::Core::Transform VKFW::Tools::SceneLoader::load_transform(tinyxml2::XMLElement* obj) {
    tinyxml2::XMLElement* transformElement = obj->FirstChildElement("Transform");

    Core::Transform transform = {};
    if (transformElement)
    {
        tinyxml2::XMLElement* scaleElement    = transformElement->FirstChildElement("scale");
        tinyxml2::XMLElement* positionElement = transformElement->FirstChildElement("translate");
        tinyxml2::XMLElement* rotationElement = transformElement->FirstChildElement("rotate");

        if (positionElement)
        {
            transform.position.x = positionElement->FloatAttribute("x");
            transform.position.y = positionElement->FloatAttribute("y");
            transform.position.z = positionElement->FloatAttribute("z");
        }

        if (rotationElement)
        {
            transform.rotation.x = math::radians(rotationElement->FloatAttribute("x"));
            transform.rotation.y = math::radians(rotationElement->FloatAttribute("y"));
            transform.rotation.z = math::radians(rotationElement->FloatAttribute("z"));
        }

        if (scaleElement)
        {
            transform.scale.x = scaleElement->FloatAttribute("x");
            transform.scale.y = scaleElement->FloatAttribute("y");
            transform.scale.z = scaleElement->FloatAttribute("z");
        }
    };
    return transform;
}

void VKFW::Tools::SceneLoader::save_transform(const Core::Transform& transform, tinyxml2::XMLElement* parentElement) {
    tinyxml2::XMLDocument* doc = parentElement->GetDocument();

    if (!doc)
        return;

    tinyxml2::XMLElement* transformElement = doc->NewElement("Transform");

    // Save position
    tinyxml2::XMLElement* positionElement = doc->NewElement("translate");
    positionElement->SetAttribute("x", transform.position.x);
    positionElement->SetAttribute("y", transform.position.y);
    positionElement->SetAttribute("z", transform.position.z);
    transformElement->InsertEndChild(positionElement);

    // Save rotation
    tinyxml2::XMLElement* rotationElement = doc->NewElement("rotate");
    rotationElement->SetAttribute("x", math::degrees(transform.rotation.x));
    rotationElement->SetAttribute("y", math::degrees(transform.rotation.y));
    rotationElement->SetAttribute("z", math::degrees(transform.rotation.z));
    transformElement->InsertEndChild(rotationElement);

    // Save scale
    tinyxml2::XMLElement* scaleElement = doc->NewElement("scale");
    scaleElement->SetAttribute("x", transform.scale.x);
    scaleElement->SetAttribute("y", transform.scale.y);
    scaleElement->SetAttribute("z", transform.scale.z);
    transformElement->InsertEndChild(scaleElement);

    // Attach the transform element to the parent
    parentElement->InsertEndChild(transformElement);
}
void VKFW::Tools::SceneLoader::load_children(tinyxml2::XMLElement* element, Core::Object3D* const parent, std::string resourcesPath) {
    for (tinyxml2::XMLElement* meshElement = element->FirstChildElement("Mesh"); meshElement; meshElement = meshElement->NextSiblingElement("Mesh"))
    {
        Core::Mesh* mesh = new Core::Mesh();
        /*
        LOAD GEOMETRY
        */
        std::string meshType = std::string(meshElement->Attribute("type")); // "file" is expected
        if (meshType == "file")
        {
            tinyxml2::XMLElement* filenameElement = meshElement->FirstChildElement("Filename");
            if (filenameElement)
            {
                Loaders::load_3D_file(mesh, resourcesPath + std::string(filenameElement->Attribute("value")), m_asyncLoad);
            }
        }
        if (meshType == "plane")
        {
            mesh->set_geometry(Core::Geometry::create_quad());
        }
        if (meshType == "sphere")
        {
            Loaders::load_3D_file(mesh, ENGINE_RESOURCES_PATH "meshes/sphere.obj", m_asyncLoad);
        }
        if (meshType == "cube")
        {
            mesh->set_geometry(Core::Geometry::create_simple_cube());
        }

        /*
        SET TRANSFORM
        */
        mesh->set_transform(load_transform(meshElement));
        /*
        SET PARAMS
        */
        if (meshElement->Attribute("name"))
            mesh->set_name(meshElement->Attribute("name"));
        // if (meshElement->BoolAttribute("alphaTest"))
        //     mesh->ray_hittable(meshElement->BoolAttribute("rayHittable"))

        tinyxml2::XMLElement* materialElement = meshElement->FirstChildElement("Material");
        Core::IMaterial*      mat             = nullptr;
        if (materialElement)
        {
            std::string materialType = std::string(materialElement->Attribute("type"));
            if (materialType == "physical")
            {
                mat                              = new Core::PhysicalMaterial();
                Core::PhysicalMaterial* material = static_cast<Core::PhysicalMaterial*>(mat);

                if (materialElement->FirstChildElement("albedo"))
                {
                    Vec4 albedo = Vec4(0.0);
                    albedo.r    = materialElement->FirstChildElement("albedo")->FloatAttribute("r");
                    albedo.g    = materialElement->FirstChildElement("albedo")->FloatAttribute("g");
                    albedo.b    = materialElement->FirstChildElement("albedo")->FloatAttribute("b");
                    albedo.a    = materialElement->FirstChildElement("albedo")->FloatAttribute("a");
                    material->set_albedo(albedo);
                }

                if (materialElement->FirstChildElement("roughness"))
                    material->set_roughness(materialElement->FirstChildElement("roughness")->FloatAttribute("value"));
                if (materialElement->FirstChildElement("metalness"))
                    material->set_metalness(materialElement->FirstChildElement("metalness")->FloatAttribute("value"));

                if (materialElement->FirstChildElement("tile"))
                {
                    Vec2 tiling = Vec2(1, 1);
                    tiling.x    = materialElement->FirstChildElement("tile")->FloatAttribute("u");
                    tiling.y    = materialElement->FirstChildElement("tile")->FloatAttribute("v");
                    material->set_tile(tiling);
                }
                if (materialElement->FirstChildElement("emission"))
                {
                    Vec3 emission = Vec3(0.0);
                    emission.r    = materialElement->FirstChildElement("emission")->FloatAttribute("r");
                    emission.g    = materialElement->FirstChildElement("emission")->FloatAttribute("g");
                    emission.b    = materialElement->FirstChildElement("emission")->FloatAttribute("b");
                    material->set_emissive_color(emission);
                    if (materialElement->FirstChildElement("emissionPower"))
                        material->set_emission_intensity(materialElement->FirstChildElement("emissionPower")->FloatAttribute("value"));
                }

                // Textures
                tinyxml2::XMLElement* texturesElement = materialElement->FirstChildElement("Textures");
                if (texturesElement)
                {
                    tinyxml2::XMLElement* albedoTexture = texturesElement->FirstChildElement("albedo");
                    if (albedoTexture)
                    {
                        Core::ITexture* texture = new Core::TextureLDR();
                        Loaders::load_texture(texture, resourcesPath + std::string(albedoTexture->Attribute("path")), TEXTURE_FORMAT_SRGB, m_asyncLoad);
                        material->set_albedo_texture(texture);
                    }
                    tinyxml2::XMLElement* normalTexture = texturesElement->FirstChildElement("normals");
                    if (normalTexture)
                    {
                        Core::ITexture* texture = new Core::TextureLDR();
                        Loaders::load_texture(texture, resourcesPath + std::string(normalTexture->Attribute("path")), TEXTURE_FORMAT_UNORM, m_asyncLoad);
                        material->set_normal_texture(texture);
                    }
                    tinyxml2::XMLElement* roughTexture = texturesElement->FirstChildElement("roughness");
                    if (roughTexture)
                    {
                        Core::ITexture* texture = new Core::TextureLDR();
                        Loaders::load_texture(texture, resourcesPath + std::string(roughTexture->Attribute("path")), TEXTURE_FORMAT_UNORM, m_asyncLoad);
                        material->set_roughness_texture(texture);
                    }
                    tinyxml2::XMLElement* metalTexture = texturesElement->FirstChildElement("metalness");
                    if (metalTexture)
                    {
                        Core::ITexture* texture = new Core::TextureLDR();
                        Loaders::load_texture(texture, resourcesPath + std::string(metalTexture->Attribute("path")), TEXTURE_FORMAT_UNORM, m_asyncLoad);
                        material->set_metallic_texture(texture);
                    }
                    tinyxml2::XMLElement* aoTexture = texturesElement->FirstChildElement("ao");
                    if (aoTexture)
                    {
                        Core::ITexture* texture = new Core::TextureLDR();
                        Loaders::load_texture(texture, resourcesPath + std::string(aoTexture->Attribute("path")), TEXTURE_FORMAT_UNORM, m_asyncLoad);
                        material->set_occlusion_texture(texture);
                    }
                    tinyxml2::XMLElement* emissiveTexture = texturesElement->FirstChildElement("emission");
                    if (emissiveTexture)
                    {
                        Core::ITexture* texture = new Core::TextureLDR();
                        Loaders::load_texture(texture, resourcesPath + std::string(emissiveTexture->Attribute("path")), TEXTURE_FORMAT_SRGB, m_asyncLoad);
                        material->set_emissive_texture(texture);
                    }
                    tinyxml2::XMLElement* maskTexture = texturesElement->FirstChildElement("mask");
                    if (maskTexture)
                    {
                        Core::ITexture* texture = new Core::TextureLDR();
                        Loaders::load_texture(texture, resourcesPath + std::string(maskTexture->Attribute("path")), TEXTURE_FORMAT_UNORM, m_asyncLoad);
                        if (std::string(maskTexture->Attribute("type")) == "UnityHDRP")
                            material->set_mask_texture(texture, MaskType::UNITY_HDRP);
                        if (std::string(maskTexture->Attribute("type")) == "Unreal")
                            material->set_mask_texture(texture, MaskType::UNREAL_ENGINE);
                        if (std::string(maskTexture->Attribute("type")) == "UnityURP")
                            material->set_mask_texture(texture, MaskType::UNITY_URP);
                    }
                }
            }
            if (materialType == "phong")
            {
            }
            if (materialType == "unlit")
            {
                mat                           = new Core::UnlitMaterial();
                Core::UnlitMaterial* material = static_cast<Core::UnlitMaterial*>(mat);

                if (materialElement->FirstChildElement("color"))
                {
                    Vec4 color = Vec4(0.0);
                    color.r    = materialElement->FirstChildElement("color")->FloatAttribute("r");
                    color.g    = materialElement->FirstChildElement("color")->FloatAttribute("g");
                    color.b    = materialElement->FirstChildElement("color")->FloatAttribute("b");
                    color.a    = materialElement->FirstChildElement("color")->FloatAttribute("a");

                    material->set_color(color);
                }
                if (materialElement->FirstChildElement("tile"))
                {
                    Vec2 tiling = Vec2(1, 1);
                    tiling.x    = materialElement->FirstChildElement("tile")->FloatAttribute("u");
                    tiling.y    = materialElement->FirstChildElement("tile")->FloatAttribute("v");
                    material->set_tile(tiling);
                }
                tinyxml2::XMLElement* texturesElement = materialElement->FirstChildElement("Textures");
                if (texturesElement)
                {
                    tinyxml2::XMLElement* albedoTexture = texturesElement->FirstChildElement("color");
                    if (albedoTexture)
                    {
                        Core::ITexture* texture = new Core::TextureLDR();
                        Loaders::load_texture(texture, resourcesPath + std::string(albedoTexture->Attribute("path")), TEXTURE_FORMAT_SRGB, m_asyncLoad);
                        material->set_color_texture(texture);
                    }
                }
            }
            if (materialType == "hair")
            {
            }
        }
        if (mat)
            mat->enable_alpha_test(meshElement->BoolAttribute("alphaTest"));
        mesh->add_material(mat ? mat : new Core::PhysicalMaterial(Vec4(0.5, 0.5, 0.5, 1.0)));

        if (meshElement->FirstChildElement("Children"))
            load_children(meshElement->FirstChildElement("Children"), mesh, resourcesPath);

        parent->add_child(mesh);
    }
    // Load lights
    for (tinyxml2::XMLElement* lightElement = element->FirstChildElement("Light"); lightElement; lightElement = lightElement->NextSiblingElement("Light"))
    {
        Core::Light* light     = nullptr;
        std::string  lightType = std::string(lightElement->Attribute("type"));
        if (lightType == "point")
        {
            light                        = new Core::PointLight();
            Core::PointLight* pointlight = static_cast<Core::PointLight*>(light);

            if (lightElement->FirstChildElement("influence"))
                pointlight->set_area_of_effect(lightElement->FirstChildElement("influence")->FloatAttribute("value"));
        }

        if (lightType == "directional")
        {
            light                            = new Core::DirectionalLight(Vec3(0.0));
            Core::DirectionalLight* dirLight = static_cast<Core::DirectionalLight*>(light);

            if (lightElement->FirstChildElement("direction"))
            {

                Vec3 dir = Vec3(1.0);
                dir.x    = lightElement->FirstChildElement("direction")->FloatAttribute("x");
                dir.y    = lightElement->FirstChildElement("direction")->FloatAttribute("y");
                dir.z    = lightElement->FirstChildElement("direction")->FloatAttribute("z");

                dirLight->set_direction(dir);
            }
        }
        if (lightType == "spot")
        {
        }
        if (light)
        {
            if (lightElement->Attribute("name"))
                light->set_name(lightElement->Attribute("name"));

            light->set_transform(load_transform(lightElement));

            if (lightElement->FirstChildElement("intensity"))
                light->set_intensity(lightElement->FirstChildElement("intensity")->FloatAttribute("value"));

            if (lightElement->FirstChildElement("color"))
            {
                Vec3 color = Vec3(0.0);
                color.r    = lightElement->FirstChildElement("color")->FloatAttribute("r");
                color.g    = lightElement->FirstChildElement("color")->FloatAttribute("g");
                color.b    = lightElement->FirstChildElement("color")->FloatAttribute("b");

                light->set_color(color);
            }
            // Shadows
            tinyxml2::XMLElement* shadowElement = lightElement->FirstChildElement("Shadow");
            if (shadowElement)
            {
                std::string shadowType = std::string(shadowElement->Attribute("type"));
                if (shadowType == "rt")
                {
                    light->set_shadow_type(ShadowType::RAYTRACED_SHADOW);
                    if (shadowElement->FirstChildElement("samples"))
                        light->set_shadow_ray_samples(shadowElement->FirstChildElement("samples")->IntAttribute("value"));
                    if (shadowElement->FirstChildElement("area"))
                        light->set_area(shadowElement->FirstChildElement("area")->FloatAttribute("value"));
                }
                if (shadowType == "classic")
                {
                    light->set_shadow_type(ShadowType::BASIC_SHADOW);
                    /*
                    TBD
                    */
                }
                if (shadowType == "vsm")
                {
                    light->set_shadow_type(ShadowType::VSM_SHADOW);
                    /*
                    TBD
                    */
                }
            }

            if (lightElement->FirstChildElement("Children"))
                load_children(lightElement->FirstChildElement("Children"), light, resourcesPath);

            parent->add_child(light);
        }
    }
}

void VKFW::Tools::SceneLoader::save_children(tinyxml2::XMLElement* parentElement, Core::Object3D* const parent) {

    tinyxml2::XMLDocument* doc = parentElement->GetDocument();

    for (Core::Object3D* child : parent->get_children())
    {
        if (child->get_type() == ObjectType::MESH)
        {
            Core::Mesh* mesh = static_cast<Core::Mesh*>(child);

            tinyxml2::XMLElement* meshElement = doc->NewElement("Mesh");
            if (mesh->get_file_route() != "")
                meshElement->SetAttribute("type", "file");
            else
                meshElement->SetAttribute("type", "primitive");

            tinyxml2::XMLElement* filenameElement = doc->NewElement("Filename");
            filenameElement->SetAttribute("value", mesh->get_file_route().c_str());
            meshElement->InsertEndChild(filenameElement);
            // Save transform
            Core::Transform transform = mesh->get_transform();
            save_transform(transform, meshElement);

            meshElement->SetAttribute("name", mesh->get_name().c_str());

            // // Save material
            Core::IMaterial* mat = mesh->get_material();
            if (mat)
            {
                tinyxml2::XMLElement* materialElement = doc->NewElement("Material");
                materialElement->SetAttribute("type", mat->get_shaderpass_ID().c_str());

                if (mat->get_shaderpass_ID() == "physical")
                {
                    Core::PhysicalMaterial* material = static_cast<Core::PhysicalMaterial*>(mat);
                    // Save Albedo
                    tinyxml2::XMLElement* albedoElement = doc->NewElement("albedo");
                    Vec3                  albedo        = material->get_albedo();
                    albedoElement->SetAttribute("r", albedo.r);
                    albedoElement->SetAttribute("g", albedo.g);
                    albedoElement->SetAttribute("b", albedo.b);
                    materialElement->InsertEndChild(albedoElement);
                    tinyxml2::XMLElement* opElement = doc->NewElement("opacity");
                    opElement->SetAttribute("value", material->get_opacity());
                    materialElement->InsertEndChild(opElement);

                    // Save Roughness
                    tinyxml2::XMLElement* roughnessElement = doc->NewElement("roughness");
                    roughnessElement->SetAttribute("value", material->get_roughness());
                    materialElement->InsertEndChild(roughnessElement);

                    // Save Metallic
                    tinyxml2::XMLElement* metallicElement = doc->NewElement("metallic");
                    metallicElement->SetAttribute("value", material->get_metalness());
                    materialElement->InsertEndChild(metallicElement);

                    // Save Emission
                    tinyxml2::XMLElement* emissionElement = doc->NewElement("emission");
                    Vec3                  emission        = material->get_emissive_color();
                    emissionElement->SetAttribute("r", emission.r);
                    emissionElement->SetAttribute("g", emission.g);
                    emissionElement->SetAttribute("b", emission.b);
                    materialElement->InsertEndChild(emissionElement);

                    // Save Textures
                    if (!material->get_textures().empty())
                    {
                        tinyxml2::XMLElement* texturesElement = doc->NewElement("Textures");

                        if (material->get_albedo_texture())
                        {
                            tinyxml2::XMLElement* albedoTextureElement = doc->NewElement("albedo");
                            albedoTextureElement->SetAttribute("path", material->get_albedo_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(albedoTextureElement);
                        }

                        if (material->get_normal_texture())
                        {
                            tinyxml2::XMLElement* normalsTextureElement = doc->NewElement("normals");
                            normalsTextureElement->SetAttribute("path", material->get_normal_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(normalsTextureElement);
                        }
                        if (material->get_roughness_texture())
                        {
                            tinyxml2::XMLElement* normalsTextureElement = doc->NewElement("roughness");
                            normalsTextureElement->SetAttribute("path", material->get_roughness_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(normalsTextureElement);
                        }
                        if (material->get_metallic_texture())
                        {
                            tinyxml2::XMLElement* normalsTextureElement = doc->NewElement("metalness");
                            normalsTextureElement->SetAttribute("path", material->get_metallic_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(normalsTextureElement);
                        }

                        if (material->get_emissive_texture())
                        {
                            tinyxml2::XMLElement* emissionTextureElement = doc->NewElement("emission");
                            emissionTextureElement->SetAttribute("path", material->get_emissive_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(emissionTextureElement);
                        }

                        materialElement->InsertEndChild(texturesElement);
                    }
                }
                // Append to parent element
                meshElement->InsertEndChild(materialElement);
            }

            // Recursively save children
            tinyxml2::XMLElement* childrenElement = doc->NewElement("Children");
            save_children(childrenElement, mesh);
            meshElement->InsertEndChild(childrenElement);

            parentElement->InsertEndChild(meshElement);
        }
        if (child->get_type() == ObjectType::LIGHT)
        {
            Core::Light* light = static_cast<Core::Light*>(child);

            tinyxml2::XMLElement* lightElement     = doc->NewElement("Light");
            tinyxml2::XMLElement* directionElement = nullptr;
            tinyxml2::XMLElement* influenceElement = nullptr;
            switch (light->get_light_type())
            {
            case LightType::POINT: {
                lightElement->SetAttribute("type", "point");
                auto* influenceElement = doc->NewElement("influence");
                influenceElement->SetAttribute("value", static_cast<Core::PointLight*>(light)->get_area_of_effect());
                lightElement->InsertEndChild(influenceElement);
                break;
            }

            case LightType::DIRECTIONAL: {
                lightElement->SetAttribute("type", "directional");
                auto* directionElement = doc->NewElement("direction");
                Vec3  direction        = static_cast<Core::DirectionalLight*>(light)->get_direction();
                directionElement->SetAttribute("x", direction.x);
                directionElement->SetAttribute("y", direction.y);
                directionElement->SetAttribute("z", direction.z);
                lightElement->InsertEndChild(directionElement);
                break;
            }

            case LightType::SPOT: {
                lightElement->SetAttribute("type", "spot");
                break;
            }
            case LightType::AREA: {
            }
            }
            lightElement->SetAttribute("name", light->get_name().c_str());

            // Save transform
            Core::Transform transform = light->get_transform();
            save_transform(transform, lightElement);

            // Save Intensity
            tinyxml2::XMLElement* intensityElement = doc->NewElement("intensity");
            intensityElement->SetAttribute("value", light->get_intensity());
            lightElement->InsertEndChild(intensityElement);

            // Save Color
            tinyxml2::XMLElement* colorElement = doc->NewElement("color");
            Vec3                  color        = light->get_color();
            colorElement->SetAttribute("r", color.r);
            colorElement->SetAttribute("g", color.g);
            colorElement->SetAttribute("b", color.b);
            lightElement->InsertEndChild(colorElement);

            if (light->get_cast_shadows())
            {
                tinyxml2::XMLElement* shadowElement = doc->NewElement("Shadow");
                switch (light->get_shadow_type())
                {
                case ShadowType::BASIC_SHADOW:
                    shadowElement->SetAttribute("type", "basic");
                    break;
                case ShadowType::VSM_SHADOW:
                    shadowElement->SetAttribute("type", "vsm");
                    break;
                case ShadowType::RAYTRACED_SHADOW:
                    shadowElement->SetAttribute("type", "rt");
                    break;
                }

                tinyxml2::XMLElement* samplesElement = doc->NewElement("samples");
                samplesElement->SetAttribute("value", light->get_shadow_ray_samples());
                shadowElement->InsertEndChild(samplesElement);

                tinyxml2::XMLElement* areaElement = doc->NewElement("area");
                areaElement->SetAttribute("value", light->get_area());
                shadowElement->InsertEndChild(areaElement);

                lightElement->InsertEndChild(shadowElement);
            }

            // Recursively save children
            tinyxml2::XMLElement* childrenElement = doc->NewElement("Children");
            save_children(childrenElement, light);
            lightElement->InsertEndChild(childrenElement);

            parentElement->InsertEndChild(lightElement);
        }
    }
}
void VKFW::Tools::SceneLoader::load_scene(Core::Scene* const scene, const std::string fileName) {

    if (!scene)
        throw VKFW_Exception("Scene is null pointer");

    tinyxml2::XMLDocument doc;
    doc.LoadFile(fileName.c_str());

    std::string resources = "";
    if (doc.FirstChildElement("Scene")->FirstChildElement("Resources"))
    {
        resources = std::string(doc.FirstChildElement("Scene")->FirstChildElement("Resources")->Attribute("path"));
    }

    // Load the camera
    tinyxml2::XMLElement* cameraElement = doc.FirstChildElement("Scene")->FirstChildElement("Camera");
    if (cameraElement)
    {
        Core::Camera* camera = new Core::Camera();
        /*
        SET TRANSFORM
        */
        camera->set_transform(load_transform(cameraElement));
        camera->set_rotation({-90, 0, 0}, true);
        /*
        SET PARAMS
        */
        camera->set_far(cameraElement->FloatAttribute("far", 100.0f));
        camera->set_near(cameraElement->FloatAttribute("near", 0.1f));
        camera->set_field_of_view(cameraElement->FloatAttribute("fov", 75.0f));
        scene->add(camera);
    }

    // Load Hierarqy of children
    load_children(doc.FirstChildElement("Scene"), scene, resources);

    // Ambient
    tinyxml2::XMLElement* ambientElement = doc.FirstChildElement("Scene")->FirstChildElement("Enviroment");
    if (ambientElement)
    {
        std::string ambientType = std::string(ambientElement->Attribute("type"));
        if (ambientType == "constant")
        {
            if (ambientElement->FirstChildElement("intensity"))
                scene->set_ambient_intensity(ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
            if (ambientElement->FirstChildElement("color"))
            {
                Vec3 color = Vec3(0.0);
                color.r    = ambientElement->FirstChildElement("color")->FloatAttribute("r");
                color.g    = ambientElement->FirstChildElement("color")->FloatAttribute("g");
                color.b    = ambientElement->FirstChildElement("color")->FloatAttribute("b");

                scene->set_ambient_color(color);
            }
        }
        if (ambientType == "HDRi")
        {
            tinyxml2::XMLElement* filenameElement = ambientElement->FirstChildElement("Filename");
            if (filenameElement)
            {
                if (ambientElement->FirstChildElement("color"))
                {
                    Vec3 color = Vec3(0.0);
                    color.r    = ambientElement->FirstChildElement("color")->FloatAttribute("r");
                    color.g    = ambientElement->FirstChildElement("color")->FloatAttribute("g");
                    color.b    = ambientElement->FirstChildElement("color")->FloatAttribute("b");

                    scene->set_ambient_color(color);
                }
                Core::TextureHDR* envMap = new Core::TextureHDR();
                Tools::Loaders::load_texture(envMap, resources + std::string(filenameElement->Attribute("value")), TEXTURE_FORMAT_HDR, m_asyncLoad);
                Core::Skybox* sky = new Core::Skybox(envMap);
                sky->set_sky_type(EnviromentType::IMAGE_BASED_ENV);
                if (ambientElement->FirstChildElement("intensity"))
                {
                    sky->set_color_intensity(ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
                    scene->set_ambient_intensity(ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
                }
                scene->set_skybox(sky);
            }
        }
        if (ambientType == "procedural")
        {

            Core::Skybox* sky = new Core::Skybox();
            sky->set_sky_type(EnviromentType::PROCEDURAL_ENV);
            if (ambientElement->FirstChildElement("intensity"))
            {
                sky->set_color_intensity(ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
                scene->set_ambient_intensity(ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
            }
            scene->set_skybox(sky);
        }
    }

    else
    {
        scene->set_ambient_color(Vec3(0.0));
        scene->set_ambient_intensity(0.0f);
    }
}

void VKFW::Tools::SceneLoader::save_scene(Core::Scene* const scene, const std::string fileName) {
    if (!scene)
    {
        throw VKFW_Exception("Scene is null pointer");
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* sceneElement = doc.NewElement("Scene");

    doc.InsertFirstChild(sceneElement);

    tinyxml2::XMLElement* resourcesElement = doc.NewElement("Resources");
    resourcesElement->SetAttribute("path", "");
    sceneElement->InsertEndChild(resourcesElement);

    // Save camera
    Core::Camera* camera = scene->get_active_camera();
    if (camera)
    {
        tinyxml2::XMLElement* cameraElement = doc.NewElement("Camera");

        Core::Transform transform = camera->get_transform();
        save_transform(transform, cameraElement);
        cameraElement->SetAttribute("far", camera->get_far());
        cameraElement->SetAttribute("near", camera->get_near());
        cameraElement->SetAttribute("fov", camera->get_field_of_view());

        sceneElement->InsertEndChild(cameraElement);
    }

    // Recursively save children
    save_children(sceneElement, scene); // Assume get_root() gets the root object

    // Save environment settings
    tinyxml2::XMLElement* environmentElement = doc.NewElement("Enviroment");
    Vec3                  ambientColor       = scene->get_ambient_color();
    float                 ambientIntensity   = scene->get_ambient_intensity();

    if (scene->get_skybox())
    {
        environmentElement->SetAttribute("type", "skybox");

        tinyxml2::XMLElement* filenameElement = doc.NewElement("Filename");
        filenameElement->SetAttribute("value", scene->get_skybox()->get_enviroment_map()->get_file_route().c_str());
        environmentElement->InsertEndChild(filenameElement);
    } else
    {
        environmentElement->SetAttribute("type", "constant");

        tinyxml2::XMLElement* colorElement = doc.NewElement("color");
        colorElement->SetAttribute("r", ambientColor.r);
        colorElement->SetAttribute("g", ambientColor.g);
        colorElement->SetAttribute("b", ambientColor.b);
        environmentElement->InsertEndChild(colorElement);
    }

    tinyxml2::XMLElement* intensityElement = doc.NewElement("intensity");
    intensityElement->SetAttribute("value", ambientIntensity);
    environmentElement->InsertEndChild(intensityElement);

    sceneElement->InsertEndChild(environmentElement);

    // Save the XML to file
    doc.SaveFile(fileName.c_str());
}