#include <engine/utilities/widgets.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Panel::render(ImVec2 extent)
{
    m_pixelExtent = {m_extent.x * extent.x, m_extent.y * extent.y};
    ImGui::SetNextWindowPos({m_position.x * extent.x, m_position.y * extent.y}, (ImGuiWindowFlags)m_flags == ImGuiWindowFlags_NoMove ? ImGuiCond_Always : ImGuiCond_Once);
    ImGui::SetNextWindowSize({m_extent.x * extent.x, m_extent.y * extent.y}, (ImGuiWindowFlags)m_flags == ImGuiWindowFlags_NoMove ? ImGuiCond_Always : ImGuiCond_Once);
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
    ImGui::SeparatorText("Ambient Light");
    ImGui::Spacing();
    Vec3 ambientColor = m_scene->get_ambient_color();
    if (ImGui::ColorEdit3("Ambient Color", (float *)&ambientColor))
    {
        m_scene->set_ambient_color(ambientColor);
    }
    float ambientIntensity = m_scene->get_ambient_intensity();
    if (ImGui::DragFloat("Intensity", &ambientIntensity, 0.05f, 0.0f, 1.0f))
    {
        m_scene->set_ambient_intensity(ambientIntensity);
    }
    ImGui::Spacing();
    ImGui::SeparatorText("Fog");
    ImGui::Spacing();
    bool useFog = m_scene->is_fog_enabled();
    if (ImGui::Checkbox("Use Exponential Fog", &useFog))
    {
        m_scene->enable_fog(useFog);
    }
    if (useFog)
    {
        Vec3 fogColor = m_scene->get_fog_color();
        if (ImGui::ColorEdit3("Fog Color", (float *)&fogColor))
        {
            m_scene->set_fog_color(fogColor);
        }
        float fogDensity = m_scene->get_fog_intensity();
        if (ImGui::DragFloat("Density", &fogDensity, .5f, 0.0f, 1000.0f))
        {
            m_scene->set_fog_intensity(fogDensity);
        }
    }
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::SeparatorText("SSAO");
    ImGui::Spacing();
    bool enableSSAO = m_scene->is_ssao_enabled();
    if (ImGui::Checkbox("Enable SSAO", &enableSSAO))
    {
        m_scene->enable_ssao(enableSSAO);
    }
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
        m_object->set_position(Vec3(position[0], position[1], position[2]));
    };
    float rotation[3] = {(m_object->get_rotation().x * 180) / PI,
                         (m_object->get_rotation().y * 180) / PI,
                         (m_object->get_rotation().z * 180) / PI};
    if (ImGui::DragFloat3("Rotation", rotation, 0.1f))
    {
        m_object->set_rotation(Vec3(rotation[0], rotation[1], rotation[2]));
    };
    float scale[3] = {m_object->get_scale().x,
                      m_object->get_scale().y,
                      m_object->get_scale().z};
    if (ImGui::DragFloat3("Scale", scale, 0.1f))
    {
        m_object->set_scale(Vec3(scale[0], scale[1], scale[2]));
    };
    if (m_object->get_type() == ObjectType::MESH)
    {
        ImGui::SeparatorText("Mesh");
        Mesh *model = static_cast<Mesh *>(m_object);

        int faceCount = 0;
        int vertexCount = 0;
        for (size_t i = 0; i < model->get_num_geometries(); i++)
        {
            vertexCount += (int)model->get_geometry(i)->get_vertex_data().size();
            faceCount += (int)model->get_geometry(i)->get_vertex_index().size() / 3;
        }

        ImGui::BeginTable("Mesh Details", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody);

        ImGui::TableSetupColumn("Mesh", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, m_parent->get_pixel_extent().x * 0.5f);
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
        ImGui::BeginTable("Mesh Details", 1, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody);
        ImGui::TableSetupColumn("Material", ImGuiTableColumnFlags_NoHide);

        for (size_t i = 0; i < model->get_num_materials(); i++)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            std::string str = "ID " + std::to_string(i) + " - " + model->get_material(i)->get_shaderpass_ID() + " material";
            ImGui::Text(str.c_str());
            ImGui::Separator();

            bool test = model->get_material(i)->get_parameters().depthTest;
            if (ImGui::Checkbox("Depth Test", &test))
            {
                model->get_material(i)->enable_depth_test(test);
            }
            bool write = model->get_material(i)->get_parameters().depthWrite;
            if (ImGui::Checkbox("Depth Write", &write))
            {
                model->get_material(i)->enable_depth_writes(write);
            }

            bool culling = model->get_material(i)->get_parameters().faceCulling;
            if (ImGui::Checkbox("Enable face culling", &culling))
            {
                model->get_material(i)->set_enable_culling(culling);
            }
            if (culling)
            {
                const char *faceVisibility[] = {"FRONT", "BACK"};
                static int currentFaceVisibility = model->get_material(i)->get_parameters().culling;
                if (ImGui::Combo("Face", &currentFaceVisibility, faceVisibility, IM_ARRAYSIZE(faceVisibility)))
                {
                    MaterialSettings parameters = model->get_material(i)->get_parameters();
                    switch (currentFaceVisibility)
                    {
                    case 0:
                        model->get_material(i)->set_culling_type(_FRONT);
                        break;
                    case 1:
                        model->get_material(i)->set_culling_type(_BACK);
                        break;
                    }
                };
            }
            ImGui::Text("BLENDING AND OPACITY UNSUPORTED FOR NOW");
            const char *blending[] = {"NORMAL", "ADDITIVE", "CUSTOM"};
            static int currentBlending = model->get_material(i)->get_parameters().blending;
            if (ImGui::Combo("Blending function", &currentBlending, blending, IM_ARRAYSIZE(blending)))
            {
                switch (currentBlending)
                {
                case 0:
                    // model->getMaterialReference(i)->setBlending(BlendingType::NORMAL);
                    break;
                case 1:
                    // model->getMaterialReference(i)->setBlending(BlendingType::ADDITIVE);
                    break;
                case 2:
                    // model->getMaterialReference(i)->setBlending(BlendingType::CUSTOM);
                    break;
                }
            };
            ImGui::Separator();
            if (model->get_material(i)->get_shaderpass_ID() == "physical")
            {
                PhysicallyBasedMaterial *mat = static_cast<PhysicallyBasedMaterial *>(model->get_material(i));
                Vec3 albedo = mat->get_albedo();
                if (ImGui::ColorEdit3("Albedo", (float *)&albedo))
                {
                    mat->set_albedo(Vec4{albedo, 1.0f});
                };
                if (mat->get_albedo_texture())
                {
                    float albedoWeight = mat->get_albedo_weight();
                    if (ImGui::DragFloat("Albedo Text Weight", &albedoWeight, 0.05f, 0.0f, 1.0f))
                    {
                        mat->set_albedo_weight(albedoWeight);
                    }
                }
                float metallic = mat->get_metalness();
                float roughness = mat->get_roughness();
                float ao = mat->get_occlusion();
                if (mat->get_mask_type() == NO_MASK)
                {

                    if (ImGui::DragFloat("Metalness", &metallic, 0.05f, 0.0f, 1.0f))
                    {
                        mat->set_metalness(metallic);
                    }
                    if (mat->get_metallic_texture())
                    {
                        float weight = mat->get_metalness_weight();
                        if (ImGui::DragFloat("Metal. Text Weight", &weight, 0.05f, 0.0f, 1.0f))
                        {
                            mat->set_metalness_weight(weight);
                        }
                    }
                    if (ImGui::DragFloat("Roughness", &roughness, 0.05f, 0.0f, 1.0f))
                    {
                        mat->set_roughness(roughness);
                    }
                    if (mat->get_roughness_texture())
                    {
                        float weight = mat->get_roughness_weight();
                        if (ImGui::DragFloat("Rough. Text Weight", &weight, 0.05f, 0.0f, 1.0f))
                        {
                            mat->set_roughness_weight(weight);
                        }
                    }
                    if (ImGui::DragFloat("Occlusion", &ao, 0.05f, 0.0f, 1.0f))
                    {
                        mat->set_occlusion(ao);
                    }
                    if (mat->get_occlusion_texture())
                    {
                        float weight = mat->get_occlusion_weight();
                        if (ImGui::DragFloat("AO Text Weight", &weight, 0.05f, 0.0f, 1.0f))
                        {
                            mat->set_occlusion_weight(weight);
                        }
                    }
                }
                else
                {
                    switch (mat->get_mask_type())
                    {
                    case UNREAL_ENGINE:
                        break;
                        ImGui::Text("Unreal Mask");
                    case UNITY_HDRP:
                        ImGui::Text("Unity HDRP Mask");
                        break;
                    case UNITY_URP:
                        ImGui::Text("Unity URP Mask");
                        break;
                    }
                    if (mat->get_mask_texture())
                    {
                        if (ImGui::DragFloat("Metalness", &metallic, 0.05f, 0.0f, 1.0f))
                        {
                            mat->set_metalness(metallic);
                        }
                        float mweight = mat->get_metalness_weight();
                        if (ImGui::DragFloat("Metal. Text Weight", &mweight, 0.05f, 0.0f, 1.0f))
                        {
                            mat->set_metalness_weight(mweight);
                        }
                        if (ImGui::DragFloat("Roughness", &roughness, 0.05f, 0.0f, 1.0f))
                        {
                            mat->set_roughness(roughness);
                        }
                        float rweight = mat->get_roughness_weight();
                        if (ImGui::DragFloat("Rough. Text Weight", &rweight, 0.05f, 0.0f, 1.0f))
                        {
                            mat->set_roughness_weight(rweight);
                        }
                        if (ImGui::DragFloat("Occlusion", &ao, 0.05f, 0.0f, 1.0f))
                        {
                            mat->set_occlusion(ao);
                        }
                        float oweight = mat->get_occlusion_weight();
                        if (ImGui::DragFloat("AO Text Weight", &oweight, 0.05f, 0.0f, 1.0f))
                        {
                            mat->set_occlusion_weight(oweight);
                        }
                    }
                }

                ImGui::Separator();
                float tile_u = mat->get_tile().x;
                float tile_v = mat->get_tile().y;
                if (ImGui::DragFloat("Tile U", &tile_u, 0.5f, -100.0f, 100.0f))
                {
                    mat->set_tile({tile_u, mat->get_tile().y});
                }
                if (ImGui::DragFloat("Tile V", &tile_v, 0.5f, -100.0f, 100.0f))
                {
                    mat->set_tile({mat->get_tile().x, tile_v});
                }

                ImGui::Separator();
            }

            if (model->get_material(i)->get_shaderpass_ID() == "phong")
            {
                // TO DO...
            }
            if (model->get_material(i)->get_shaderpass_ID() == "unlit")
            {
                // TO DO...
            }
            ImGui::Separator();
        }
        ImGui::EndTable();
    }

    if (m_object->get_type() == ObjectType::LIGHT)
    {
        ImGui::SeparatorText("Light");

        Light *light = static_cast<Light *>(m_object);

        float intensity = light->get_intensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.005f, 0.0f, 10.0f))
            light->set_intensity(intensity);
        Vec3 color = light->get_color();
        if (ImGui::ColorEdit3("Color", (float *)&color))
        {
            light->set_color(color);
        };

        if (light->get_light_type() == LightType::POINT)
        {
            float att = static_cast<PointLight *>(light)->get_area_of_effect();
            if (ImGui::DragFloat("Area of Influence", &att, 0.005f, 0.0f, 50.0f))
                static_cast<PointLight *>(light)->set_area_of_effect(att);
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
                light->set_shadow_target(Vec3(position[0], position[1], position[2]));
            };
            ImGui::Text("Advanced Shadow Settings:");
            float bias = light->get_shadow_bias();
            if (ImGui::DragFloat("Shadow Bias", &bias, 0.0001f, 0.0f, 1.0f))
                light->set_shadow_bias(bias);

            int kernel = light->get_shadow_pcf_kernel();
            if (ImGui::DragInt("PC Filter Kernel", &kernel, 2, 3, 15))
                light->set_shadow_pcf_kernel(kernel);
        }
    }
    if (m_object->get_type() == ObjectType::CAMERA)
    {
        ImGui::SeparatorText("Camera");

        Camera *cam = static_cast<Camera *>(m_object);
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
}
VULKAN_ENGINE_NAMESPACE_END
