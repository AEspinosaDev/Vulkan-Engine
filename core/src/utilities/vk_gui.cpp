#include "engine/utilities/vk_gui.h"

namespace vke
{
    bool GUIOverlay::m_initialized = false;

    void GUIOverlay::init(VkInstance &instance, VkDevice &device, VkPhysicalDevice &gpu, VkQueue &graphicsQueue, VkRenderPass &renderPass,
                          VkFormat format, VkSampleCountFlagBits samples, GLFWwindow *window)
    {
        if (GUIOverlay::m_initialized)
            return;

        // 1: create descriptor pool for IMGUI
        VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &m_pool));

        // 2: initialize imgui library
        // this initializes the core structures of imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        // ImGui::SetCurrentContext()
        ImGui::StyleColorsDark();

        // this initializes imgui for SDL
        ImGui_ImplGlfw_InitForVulkan(window, true);

        // this initializes imgui for Vulkan
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = gpu;
        init_info.Device = device;
        init_info.Queue = graphicsQueue;
        init_info.DescriptorPool = m_pool;
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        // init_info.UseDynamicRendering = true;
        init_info.ColorAttachmentFormat = format;

        init_info.MSAASamples = samples;

        ImGui_ImplVulkan_Init(&init_info, renderPass);

        GUIOverlay::m_initialized = true;
    }

    void GUIOverlay::render()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        for (auto p : m_panels)
        {
            p->render();
            p->set_is_resized(m_resized);
        }
        ImGui::Render();

        m_resized = false;
    }

    void GUIOverlay::upload_draw_data(VkCommandBuffer &cmd)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }

    void GUIOverlay::cleanup(VkDevice &device)
    {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(device, m_pool, nullptr);
    }

    void Panel::render()
    {
        m_pixelExtent = {m_extent.x * m_parentOverlay->get_extent().x, m_extent.y * m_parentOverlay->get_extent().y};
        ImGui::SetNextWindowPos({m_position.x * m_parentOverlay->get_extent().x, m_position.y * m_parentOverlay->get_extent().y}, (ImGuiWindowFlags)m_flags == ImGuiWindowFlags_NoMove ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::SetNextWindowSize({m_extent.x * m_parentOverlay->get_extent().x, m_extent.y * m_parentOverlay->get_extent().y}, (ImGuiWindowFlags)m_flags == ImGuiWindowFlags_NoMove ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::SetNextWindowCollapsed(m_collapsed, ImGuiCond_Once);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_padding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, m_minExtent);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, m_borderSize);
        if (m_color.x > -1)
            ImGui::PushStyleColor(ImGuiCol_MenuBarBg, m_color);

        if (m_open)
        {
            if (ImGui::Begin(m_title, m_closable ? &m_open : NULL, m_collapsable ? (ImGuiWindowFlags)m_flags | ImGuiWindowFlags_NoResize : (ImGuiWindowFlags)m_flags | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
            {
                //     ImGui::SetWindowPos({m_position.x * m_parentOverlay->get_extent().x, m_position.y * m_parentOverlay->get_extent().y});
                // if (m_resized)
                // {
                //     m_resized = false;
                // }

                for (auto widget : m_children)
                    widget->render();
            }

            ImGui::End();
        }
        if (m_color.x > -1)
            ImGui::PopStyleColor();
        ImGui::PopStyleVar(4);
    }
    void TextLine::render()
    {
        switch (m_type)
        {
        case SIMPLE:
            ImGui::Text(m_text);
            break;
        case WARPED:
            ImGui::TextWrapped(m_text);
            break;
        case COLORIZED:
            ImGui::TextColored(m_color, m_text);
            break;
        case BULLET:
            ImGui::BulletText(m_text);
            break;
        }
    }
    void SceneExplorerWidget::render()
    {
        ImGui::Spacing();
        ImGui::SeparatorText("SCENE EXPLORER");

        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoBordersInBody /*| ImGuiTableFlags_BordersH*/;

        if (ImGui::BeginTable("2ways", 2, flags, ImVec2(m_parent->get_pixel_extent().x * 0.95f, m_parent->get_pixel_extent().y * 0.3f)))
        {
            ImGui::TableSetupColumn(" Name", ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 3 * 18.0f);
            ImGui::TableHeadersRow();

            // Simple storage to output a dummy file-system.
            struct MyTreeNode
            {
                Object3D *obj;
                const char *Name;
                const char *Type;
                int ChildIdx;
                int ChildCount;
                bool selected;
                static void DisplayNode(MyTreeNode *node, std::vector<MyTreeNode> all_nodes)
                {

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    const bool is_folder = (node->ChildCount > 0);
                    if (is_folder)
                    {
                        bool open = ImGui::TreeNodeEx(node->Name, ImGuiTreeNodeFlags_SpanFullWidth);
                        ImGui::TableNextColumn();
                        ImGui::TextDisabled("--");
                        if (open)
                        {
                            for (int child_n = 0; child_n < node->ChildCount; child_n++)
                                DisplayNode(&all_nodes[node->ChildIdx + child_n], all_nodes);
                            ImGui::TreePop();
                        }
                    }
                    else
                    {

                        if (ImGui::TreeNodeEx(node->obj->get_name().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth))
                        {
                            /*	node->selected = ImGui::IsItemClicked();
                                if (ImGui::IsItemClicked()) {
                                    UIManager::setSelectedObj(node->obj, true);
                                }*/
                        };

                        ImGui::TableNextColumn();

                        if (ImGui::SmallButton(node->obj->is_active() ? "true" : "false"))
                        {
                            node->obj->set_active(!node->obj->is_active());
                        }
                    }
                }
            };

            auto objs = m_scene->get_children();
            std::vector<MyTreeNode> nodes;
            int counter = 0;
            for (auto obj : objs)
            {
                auto tpe = obj->get_type();

                ImGui::PushID(counter);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                std::string name = obj->get_name();

                if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth))
                {
                    // node->selected = ImGui::IsItemClicked();
                    if (ImGui::IsItemClicked())
                    {
                        // if (m_selectedObject)
                        //     UIManager::m_SelectedObject->selected = false;
                        m_selectedObject = obj;
                    }
                };

                ImGui::TableNextColumn();
                if (ImGui::Button(obj->is_active() ? "true" : "false"))
                {
                    obj->set_active(!obj->is_active());
                }
                ImGui::PopID();

                counter++;
            }

            ImGui::EndTable();
        }
        ImGui::Spacing();
        ImGui::Separator();
    }
    void Profiler::render()
    {
        ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
    void Space::render()
    {
        ImGui::Spacing();
    }
    void Separator::render()
    {
        !m_text ? ImGui::Separator() : ImGui::SeparatorText(m_text);
    }

    void ObjectExplorerWidget::render()
    {
        if (!m_object)
        {
            ImGui::Text("Select an object in the scene explorer");
            return;
        }

        std::string str = m_object->get_name();
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        ImGui::BulletText(str.c_str());

        ImGui::SeparatorText("Transform");
        const float PI = 3.14159265359f;

        float position[3] = {m_object->get_position().x,
                             m_object->get_position().y,
                             m_object->get_position().z};
        if (ImGui::DragFloat3("Position", position, 0.1f))
        {
            m_object->set_position(glm::vec3(position[0], position[1], position[2]));
        };
        float rotation[3] = {(m_object->get_rotation().x * 180) / PI,
                             (m_object->get_rotation().y * 180) / PI,
                             (m_object->get_rotation().z * 180) / PI};
        if (ImGui::DragFloat3("Rotation", rotation, 0.1f))
        {
            m_object->set_rotation(glm::vec3((rotation[0] * PI) / 180, (rotation[1] * PI) / 180, (rotation[2] * PI) / 180));
        };
        float scale[3] = {m_object->get_scale().x,
                          m_object->get_scale().y,
                          m_object->get_scale().z};
        if (ImGui::DragFloat3("Scale", scale, 0.1f))
        {
            m_object->set_scale(glm::vec3(scale[0], scale[1], scale[2]));
        };
        if (m_object->get_type() == ObjectType::MESH)
        {
            ImGui::SeparatorText("Mesh");
            Mesh *model = dynamic_cast<Mesh *>(m_object);

            int faceCount = 0;
            int vertexCount = 0;
            for (size_t i = 0; i < model->get_num_geometries(); i++)
            {
                vertexCount += (int)model->get_geometry(i)->get_vertex_data().size();
                faceCount += (int)model->get_geometry(i)->get_vertex_index().size() / 3;
            }

            ImGui::BeginTable("Mesh Details", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody);

            ImGui::TableSetupColumn("Mesh", ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, m_parent->get_extent().x * 0.5f);
            // ImGui::TableSetupColumn("");

            // ImGui::table

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextWrapped("File route");
            ImGui::TableNextColumn();
            ImGui::Text(model->get_file_route().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Total geometries");
            ImGui::TableNextColumn();
            ImGui::Text(std::to_string(model->get_num_geometries()).c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Total face count");
            ImGui::TableNextColumn();
            ImGui::Text(std::to_string(faceCount).c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Total vertex count");
            ImGui::TableNextColumn();
            ImGui::Text(std::to_string(vertexCount).c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Instanced");
            ImGui::TableNextColumn();
            ImGui::Text("False");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Receive shadows");
            ImGui::TableNextColumn();
            bool shadows = model->get_cast_shadows();
            if (ImGui::Checkbox("", &shadows))
            {
                model->set_cast_shadows(shadows);
            };

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Affected by fog");
            ImGui::TableNextColumn();
            bool fog = model->is_affected_by_fog();
            if (ImGui::Checkbox("", &fog))
            {
                model->set_affected_by_fog(fog);
            };

            ImGui::EndTable();

            ImGui::SeparatorText("Material");
            ImGui::BeginTable("Mesh Details", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody);
            ImGui::TableSetupColumn("Material", ImGuiTableColumnFlags_NoHide);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Total materials");
            ImGui::TableNextColumn();
            ImGui::Text(std::to_string(model->get_num_materials()).c_str());
            ImGui::EndTable();

            //     for (size_t i = 0; i < model->getMesh()->getNumberOfMeshes(); i++)
            //     {
            //         ImGui::TableNextRow();
            //         ImGui::TableNextColumn();
            //         std::string str = "ID " + std::to_string(i) + " - " + model->getMaterialReference(i)->getShaderNameID();
            //         ImGui::Text(str.c_str());
            //         ImGui::Separator();
            //         bool transparent = model->getMaterialReference(i)->getTransparency();
            //         if (ImGui::Checkbox("Transparent", &transparent))
            //         {
            //             model->getMaterialReference(i)->setTransparency(transparent);
            //         }
            //         const char *faceVisibility[] = {"FRONT", "BACK", "BOTH"};
            //         static int currentFaceVisibility = model->getMaterialReference(i)->getParameters().faceVisibility;
            //         if (ImGui::Combo("Face visibility", &currentFaceVisibility, faceVisibility, IM_ARRAYSIZE(faceVisibility)))
            //         {
            //             switch (currentFaceVisibility)
            //             {
            //             case 0:
            //                 model->getMaterialReference(i)->setFaceVisibility(FaceVisibility::FRONT);
            //                 break;
            //             case 1:
            //                 model->getMaterialReference(i)->setFaceVisibility(FaceVisibility::BACK);
            //                 break;
            //             case 2:
            //                 model->getMaterialReference(i)->setFaceVisibility(FaceVisibility::BOTH);
            //                 break;
            //             }
            //         };
            //         const char *blending[] = {"NORMAL", "ADDITIVE", "CUSTOM"};
            //         static int currentBlending = model->getMaterialReference(i)->getParameters().blendingType;
            //         if (ImGui::Combo("Blending function", &currentBlending, blending, IM_ARRAYSIZE(blending)))
            //         {
            //             switch (currentBlending)
            //             {
            //             case 0:
            //                 model->getMaterialReference(i)->setBlending(BlendingType::NORMAL);
            //                 break;
            //             case 1:
            //                 model->getMaterialReference(i)->setBlending(BlendingType::ADDITIVE);
            //                 break;
            //             case 2:
            //                 model->getMaterialReference(i)->setBlending(BlendingType::CUSTOM);
            //                 break;
            //             }
            //         };

            //         if (model->getMaterialReference(i)->getShaderNameID() == "PhysicallyBasedShader")
            //         {
            //             // str += "Physical Material";
            //             // ImGui::Text("Physical Material");
            //             PhysicalMaterial *mat = dynamic_cast<PhysicalMaterial *>(model->getMaterialReference(i));
            //             glm::vec3 albedo = mat->getAlbedoColor();
            //             float metallic = mat->getMetalness();
            //             float roughness = mat->getRoughness();
            //             float ao = mat->getAO();
            //             float op = mat->getOpacity();
            //             bool receiveShadows = mat->getReceiveShadows();
            //             if (!mat->getAlbedoText())
            //             {
            //                 if (ImGui::ColorEdit3("Albedo", (float *)&albedo))
            //                 {
            //                     mat->setAlbedoColor(albedo);
            //                 };
            //             }
            //             else
            //             {
            //                 ImGui::Image(mat->getAlbedoText() ? (void *)mat->getAlbedoText()->getID() : nullptr, ImVec2(size.x * .15f, size.x * .15f));
            //                 ImGui::SameLine();
            //                 ImGui::Text("Albedo");
            //             }
            //             if (!mat->getOpacityMask())
            //             {
            //                 if (ImGui::DragFloat("Opacity", &op, 0.005f, 0.0f, 1.0f))
            //                 {
            //                     mat->setOpacity(op);
            //                 };
            //             }
            //             else
            //             {
            //                 ImGui::Image(mat->getOpacityMask() ? (void *)mat->getOpacityMask()->getID() : nullptr, ImVec2(size.x * .15f, size.x * .15f));
            //                 ImGui::SameLine();
            //                 ImGui::Text("Opacity");
            //             }
            //             switch (mat->getMaskPreset())
            //             {
            //             case _NONE:
            //                 if (!mat->getMetalnessText())
            //                 {
            //                     if (ImGui::DragFloat("Metalness", &metallic, 0.005f, 0.0f, 1.0f))
            //                         mat->setMetalness(metallic);
            //                 }
            //                 else
            //                 {
            //                     ImGui::Image(mat->getMetalnessText() ? (void *)mat->getMetalnessText()->getID() : nullptr, ImVec2(size.x * .15f, size.x * .15f));
            //                     ImGui::SameLine();
            //                     ImGui::Text("Metalness");
            //                 }
            //                 if (!mat->getRoughnessText())
            //                 {
            //                     if (ImGui::DragFloat("Roughness", &roughness, 0.005f, 0.0f, 1.0f))
            //                         mat->setRoughness(roughness);
            //                 }
            //                 else
            //                 {
            //                     ImGui::Image(mat->getRoughnessText() ? (void *)mat->getRoughnessText()->getID() : nullptr, ImVec2(size.x * .15f, size.x * .15f));
            //                     ImGui::SameLine();
            //                     ImGui::Text("Roughness");
            //                 }
            //                 if (!mat->getAOText())
            //                 {
            //                     if (ImGui::DragFloat("Ambient Occ", &ao, 0.005f, 0.0f, 1.0f))
            //                         mat->setAO(ao);
            //                 }
            //                 else
            //                 {
            //                     ImGui::Image(mat->getAOText() ? (void *)mat->getAOText()->getID() : nullptr, ImVec2(size.x * .15f, size.x * .15f));
            //                     ImGui::SameLine();
            //                     ImGui::Text("Ambient Occ");
            //                 }
            //                 break;
            //             case UNITY_HDRP:
            //                 ImGui::Image(mat->getRoughnessText() ? (void *)mat->getRoughnessText()->getID() : nullptr, ImVec2(size.x * .15f, size.x * .15f));
            //                 ImGui::SameLine();
            //                 ImGui::Text("Unity HDRP Mask");
            //                 break;
            //             case UNITY_URP:
            //                 break;
            //             case UNREAL_ENGINE_4:
            //                 ImGui::Image(mat->getRoughnessText() ? (void *)mat->getRoughnessText()->getID() : nullptr, ImVec2(size.x * .15f, size.x * .15f));
            //                 ImGui::SameLine();
            //                 ImGui::Text("Unreal 4 Mask");
            //                 break;
            //             default:
            //                 break;
            //             }

            //             if (ImGui::Checkbox("Receive Shadows", &receiveShadows))
            //             {
            //                 mat->setReceiveShadows(receiveShadows);
            //             };
            //             if (ImGui::TreeNode("Texture info"))
            //             {
            //                 ImGui::Text("Albedo");
            //                 ImGui::Image(mat->getAlbedoText() ? (void *)mat->getAlbedoText()->getID() : nullptr, ImVec2(size.x * .5f, size.x * .5f));
            //                 ImGui::Text("Normal");
            //                 ImGui::Image(mat->getNormalText() ? (void *)mat->getNormalText()->getID() : nullptr, ImVec2(size.x * .5f, size.x * .5f));
            //                 switch (mat->getMaskPreset())
            //                 {
            //                 case _NONE:
            //                     ImGui::Text("Roughness");
            //                     ImGui::Image(mat->getRoughnessText() ? (void *)mat->getRoughnessText()->getID() : nullptr, ImVec2(size.x * .5f, size.x * .5f));
            //                     ImGui::Text("Metalness");
            //                     ImGui::Image(mat->getMetalnessText() ? (void *)mat->getMetalnessText()->getID() : nullptr, ImVec2(size.x * .5f, size.x * .5f));
            //                     ImGui::Text("Ambient Occ");
            //                     ImGui::Image(mat->getAOText() ? (void *)mat->getAOText()->getID() : nullptr, ImVec2(size.x * .5f, size.x * .5f));
            //                     break;
            //                 case UNITY_HDRP:
            //                     ImGui::Text("Mask");
            //                     ImGui::Text("R:Metallic G:Occlusion A:Roughness");
            //                     ImGui::Image(mat->getRoughnessText() ? (void *)mat->getRoughnessText()->getID() : nullptr, ImVec2(size.x * .5f, size.x * .5f));
            //                     break;
            //                 case UNITY_URP:
            //                     ImGui::Text("Roughness");
            //                     ImGui::Image(mat->getRoughnessText() ? (void *)mat->getRoughnessText()->getID() : nullptr, ImVec2(size.x * .5f, size.x * .5f));
            //                     break;
            //                 case UNREAL_ENGINE_4:
            //                     ImGui::Text("Mask");
            //                     ImGui::Text("R:Occlusion G:Roughness B:Metallic");
            //                     ImGui::Image(mat->getRoughnessText() ? (void *)mat->getRoughnessText()->getID() : nullptr, ImVec2(size.x * .5f, size.x * .5f));
            //                     break;
            //                 default:
            //                     break;
            //                 }
            //                 ImGui::Text("Opacity");
            //                 ImGui::Image(mat->getOpacityMask() ? (void *)mat->getOpacityMask()->getID() : nullptr, ImVec2(size.x * .5f, size.x * .5f));

            //                 ImGui::TreePop();
            //             }
            //             ImGui::Separator();
            //         }
            //         if (model->getMaterialReference(i)->getShaderNameID() == "BasicPhongShader")
            //         {
            //         }
            //         if (model->getMaterialReference(i)->getShaderNameID() == "UnlitBasicShader")
            //         {
            //         }
            //     }
            //     ImGui::EndTable();
        }
        if (m_object->get_type() == ObjectType::LIGHT)
        {
            ImGui::SeparatorText("Light");

            Light *light = dynamic_cast<Light *>(m_object);

            float intensity = light->get_intensity();
            if (ImGui::DragFloat("Intensity", &intensity, 0.005f, 0.0f, 10.0f))
                light->set_intensity(intensity);
            glm::vec3 color = light->get_color();
            if (ImGui::ColorEdit3("Color", (float *)&color))
            {
                light->set_color(color);
            };

            if (light->get_light_type() == LightType::POINT)
            {
                float att = dynamic_cast<PointLight *>(light)->get_area_of_effect();
                if (ImGui::DragFloat("Area of Influence", &att, 0.005f, 0.0f, 50.0f))
                    dynamic_cast<PointLight *>(light)->set_area_of_effect(att);
            }
            bool castShadows = light->get_cast_shadows();

            if (ImGui::Checkbox("Cast Shadows", &castShadows))
            {
                light->set_cast_shadows(castShadows);
            };
            if (castShadows)
            {
                float shadowNear = light->get_shadow_near();
                if (ImGui::DragFloat("Shadow Near Plane", &shadowNear, 0.005f, 0.0f, 10.0f))
                    light->set_shadow_near(shadowNear);
                float shadowFar = light->get_shadow_far();
                if (ImGui::DragFloat("Shadow Far Plane", &shadowFar, 1.0f, 10.0f, 1000.0f))
                    light->set_shadow_far(shadowFar);
                float shadowFov = light->get_shadow_fov();
                if (ImGui::DragFloat("Shadow FOV", &shadowFov, 1.0f, 0.0f, 160.0f))
                    light->set_shadow_fov(shadowFov);
                float position[3] = {light->get_shadow_target().x,
                                     light->get_shadow_target().y,
                                     light->get_shadow_target().z};
                if (ImGui::DragFloat3("Shadow Target", position, 0.1f))
                {
                    light->set_shadow_target(glm::vec3(position[0], position[1], position[2]));
                };
            }
        }
        if (m_object->get_type() == ObjectType::CAMERA)
        {
            ImGui::SeparatorText("Camera");

            Camera *cam = dynamic_cast<Camera *>(m_object);
            float _far = cam->get_far();
            float _near = cam->get_near();
            float fov = cam->get_field_of_view();

            if (ImGui::DragFloat("Near", &_near, 0.05f, 0.0f, 10.0))
                cam->set_near(_near);
            if (ImGui::DragFloat("Far", &_far, 0.1f, 0.0f, 9999.0f))
                cam->set_far(_far);
            if (ImGui::DragFloat("Field of view", &fov, 0.1f, 0.0f, 160.0f))
                cam->set_field_of_view(fov);
        }
    }
    void GlobalSettingsWidget::render()
    {

        // ImGui::SeparatorText("Global settings");

        // ImGui::Checkbox("Clear Color", &m_renderer->m_settings.autoClearColor);
        // ImGui::ColorEdit3("Clear Color", (float *)&m_renderer->m_settings.clearColor); // Edit 3 floats representing a color
        // ImGui::Checkbox("Clear depth", &m_renderer->m_settings.autoClearDepth);
        // ImGui::Checkbox("Depth Test", &m_renderer->m_settings.depthTest);
        // ImGui::Checkbox("Depth Write", &m_renderer->m_settings.depthWrite);
        // ImGui::Checkbox("Gamma correction", &m_renderer->m_settings.gammaCorrection);
        // ImGui::Text("Anisotropic filter enabled");

        // // if (ImGui::Checkbox("Fog", &m_renderer->m_settings))
        // //     ;
        // // if (r->m_Settings.ppEffects.fog)
        // // {

        // //     if (ImGui::ColorEdit3("Fog Color", (float *)&r->m_Settings.ppEffects.fogColor))
        // //         ;

        // //     if (ImGui::DragFloat("Fog Density", &r->m_Settings.ppEffects.fogIntensity, 1.0f, 0.0f, 100.0f))
        // //         ;
        // //     /*
        // //         if (ImGui::DragFloat("Fog Start", &r->m_Settings.ppEffects.fogStart, 1.0f, r->m_CurrentScene->getActiveCamera()->getNear(), r->m_CurrentScene->getActiveCamera()->getFar()));
        // //         if (ImGui::DragFloat("Fog End", &r->m_Settings.ppEffects.fogEnd, 1.0f, r->m_CurrentScene->getActiveCamera()->getNear(), r->m_CurrentScene->getActiveCamera()->getFar()));
        // //     }*/
        // // }

        // const char *res[] = {"VERY LOW", "LOW", "MID", "HIGH", "ULTRA"};

        // static int res_current = 1;
        // if (ImGui::Combo("Shadow Resolution", &res_current, res, IM_ARRAYSIZE(res)))
        // {
        //     switch (res_current)
        //     {
        //     case 0:
        //         m_renderer->m_settings.shadowResolution = ShadowResolution::VERY_LOW;
        //         break;
        //     case 1:
        //         m_renderer->m_settings.shadowResolution = ShadowResolution::LOW;
        //         break;
        //     case 2:
        //         m_renderer->m_settings.shadowResolution = ShadowResolution::MEDIUM;
        //         break;
        //     case 3:
        //         m_renderer->m_settings.shadowResolution = ShadowResolution::HIGH;
        //         break;
        //     case 4:
        //         m_renderer->m_settings.shadowResolution = ShadowResolution::ULTRA;
        //         break;
        //     }
        // }

        // //     /*if (res_current != 0) {
        // //         r->m_Resources.framebuffers["msaaFBO"]->setTextureAttachmentSamples((AntialiasingType)r->m_Settings.antialiasingSamples);
        // //     }*/
        // // };
        // // ImGui::SeparatorText("Antialiasing");

        // const char *items[] = {"NONE", "MSAAx2", "MSAAx4", "MSAAx8", "MSAAx16", "OTHER"};
        // static int item_current = 4;
        // if (ImGui::Combo("Antialiasing", &item_current, items, IM_ARRAYSIZE(items)))
        // {
        //     switch (item_current)
        //     {
        //     case 0:
        //         m_renderer->m_settings.AAtype = AntialiasingType::_NONE;
        //         break;
        //     case 1:
        //         m_renderer->m_settings.AAtype = AntialiasingType::MSAA_x4;
        //         break;
        //     case 2:
        //         m_renderer->m_settings.AAtype = AntialiasingType::MSAA_x8;
        //         break;
        //     case 3:
        //         m_renderer->m_settings.AAtype = AntialiasingType::MSAA_x16;
        //         break;
        //     case 4:
        //         m_renderer->m_settings.AAtype = AntialiasingType::MSAA_x32;
        //         break;
        //     }

        //     //     if (item_current != 0)
        //     //     {
        //     //         r->m_Resources.framebuffers["msaaFBO"]->setTextureAttachmentSamples((AntialiasingType)r->m_Settings.antialiasingSamples);
        //     //     }
        //     // };
    }
}