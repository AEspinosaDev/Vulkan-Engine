#include <engine/tools/widgets.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Core;
namespace Tools {

void Panel::render(ImVec2 extent) {
    m_pixelExtent = {m_extent.x * extent.x, m_extent.y * extent.y};
    ImGui::SetNextWindowPos(
        {m_position.x * extent.x, m_position.y * extent.y}, (ImGuiWindowFlags)m_flags == ImGuiWindowFlags_NoMove ? ImGuiCond_Always : ImGuiCond_Once);
    ImGui::SetNextWindowSize(
        {m_extent.x * extent.x, m_extent.y * extent.y}, (ImGuiWindowFlags)m_flags == ImGuiWindowFlags_NoMove ? ImGuiCond_Always : ImGuiCond_Once);
    ImGui::SetNextWindowCollapsed(m_collapsed, ImGuiCond_Once);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_padding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, m_minExtent);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, m_borderSize);
    if (m_color.x > -1)
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, m_color);

    if (m_open)
    {
        if (ImGui::Begin(m_title,
                         m_closable ? &m_open : NULL,
                         m_collapsable ? (ImGuiWindowFlags)m_flags | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar
                                       : (ImGuiWindowFlags)m_flags | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            //     ImGui::SetWindowPos({m_position.x * m_parentOverlay->get_extent().x, m_position.y *
            //     m_parentOverlay->get_extent().y});
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
void TextLine::render() {
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
#pragma region SCENE EXPLORER
void ExplorerWidget::render() {

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("SaveSceneFile", "Save Scene", ".xml");
            }
            if (ImGui::MenuItem("Import Scene"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("LoadSceneFile", "Choose Scene", ".xml");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::MenuItem("Plane"))
            {
                Mesh* plane = new Mesh();
                plane->set_geometry(Geometry::create_quad());
                auto mat = new PhysicalMaterial();
                plane->add_material(mat);
                plane->set_name("Plane");
                m_scene->add(plane);
            }
            if (ImGui::MenuItem("Cube"))
            {
                Mesh* cube = new Mesh();
                Loaders::load_3D_file(cube, GET_RESOURCE_PATH("meshes/cube.obj"), false);
                auto mat = new PhysicalMaterial();
                cube->add_material(mat);
                cube->set_name("Box");
                m_scene->add(cube);
            }
            if (ImGui::MenuItem("Sphere"))
            {
                Mesh* sph = new Mesh();
                Loaders::load_3D_file(sph, GET_RESOURCE_PATH("meshes/sphere.obj"), false);
                auto mat = new PhysicalMaterial();
                sph->add_material(mat);
                sph->set_name("Sphere");
                m_scene->add(sph);
            }
            if (ImGui::MenuItem("Cylinder"))
            {

                Mesh* cyl = new Mesh();
                Loaders::load_3D_file(cyl, GET_RESOURCE_PATH("meshes/cylinder.obj"), false);
                auto mat = new PhysicalMaterial();
                cyl->add_material(mat);
                cyl->set_name("Cylinder");
                m_scene->add(cyl);
            }

            if (ImGui::MenuItem("Directional Light"))
            {
                DirectionalLight* light = new DirectionalLight(Vec3(0.0, 5.0, 5.0));
                light->set_cast_shadows(false);
                m_scene->add(light);
            }
            if (ImGui::MenuItem("Point Light"))
            {
                PointLight* light = new PointLight();
                light->set_cast_shadows(false);
                m_scene->add(light);
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Import File"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseMeshFile", "Choose a file", ".obj,.ply,.gltf");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Renderer"))
            {
                m_showRendererSettings = true;
            }
            if (ImGui::MenuItem("Procedural Sky"))
            {
                m_showSkySettings = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    if (m_showRendererSettings)
    {
        ImGui::Begin("Renderer Settings", &m_showRendererSettings, ImGuiWindowFlags_None);

        displayRendererSettings();

        // if (ImGui::Button("Close"))
        // {
        //     m_showRendererSettings = false; // Close the window when the button is pressed
        // }

        ImGui::End();
    }
    if (m_showSkySettings)
    {
        ImGui::Begin("Sky Settings", &m_showSkySettings, ImGuiWindowFlags_None);

        displaySkySettings();

        // if (ImGui::Button("Close"))
        // {
        //     m_showSkySettings = false; // Close the window when the button is pressed
        // }

        ImGui::End();
    }
    if (ImGuiFileDialog::Instance()->Display("ChooseMeshFile", 32, {800.0f, 400.0f}, {1400.0f, 1000.0f}))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

            Mesh* mesh = new Mesh();
            Loaders::load_3D_file(mesh, filePath, true);
            auto mat = new PhysicalMaterial();
            mesh->add_material(mat);
            mesh->set_name(std::filesystem::path(filePath).filename().string());
            m_scene->add(mesh);
        }

        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGuiFileDialog::Instance()->Display("SaveSceneFile", 32, {800.0f, 400.0f}, {1400.0f, 1000.0f}))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            SceneLoader sceneLoader;
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            sceneLoader.save_scene(m_scene, filePath);
        }

        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGuiFileDialog::Instance()->Display("LoadSceneFile", 32, {800.0f, 400.0f}, {1400.0f, 1000.0f}))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            SceneLoader sceneLoader;
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            sceneLoader.load_scene(m_scene, filePath);
        }

        ImGuiFileDialog::Instance()->Close();
    }
    ImGui::Spacing();

    ImGui::SeparatorText("SCENE EXPLORER");

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoBordersInBody /*| ImGuiTableFlags_BordersH*/;

    if (ImGui::BeginTable("2ways", 2, flags, ImVec2(m_parent->get_pixel_extent().x * 0.95f, m_parent->get_pixel_extent().y * 0.3f)))
    {
        ImGui::TableSetupColumn(" Name", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 3 * 18.0f);
        ImGui::TableHeadersRow();

        // Simple storage to output a dummy file-system.
        struct MyTreeNode {
            Object3D*   obj;
            const char* Name;
            const char* Type;
            int         ChildIdx;
            int         ChildCount;
            bool        selected;
            static void DisplayNode(MyTreeNode* node, std::vector<MyTreeNode> all_nodes) {
            }
        };

        auto                    objs = m_scene->get_children();
        std::vector<MyTreeNode> nodes;
        int                     counter = 0;

        for (auto obj : objs)
        {
            displayObject(obj, counter);
        }

        ImGui::EndTable();
    }
    ImGui::Spacing();
    ImGui::SeparatorText("Ambient Light");
    const char* ambientType[]  = {"CONSTANT", "ENVIROMENT"};
    static int  currentAmbient = static_cast<int>(m_scene->use_IBL());
    if (ImGui::Combo("Ambient Type", &currentAmbient, ambientType, IM_ARRAYSIZE(ambientType)))
    {
        switch (currentAmbient)
        {
        case 0:
            m_scene->use_IBL(false);
            break;
        case 1:
            m_scene->use_IBL(true);
            break;
        }
    };
    if (currentAmbient == 0)
    {
        Vec3 ambientColor = m_scene->get_ambient_color();
        if (ImGui::ColorEdit3("Ambient Color", (float*)&ambientColor))
        {
            m_scene->set_ambient_color(ambientColor);
        }
    }
    float ambientIntensity = m_scene->get_ambient_intensity();
    if (ImGui::DragFloat("Ambient Intensity", &ambientIntensity, 0.05f, 0.0f, 1.0f))
    {
        m_scene->set_ambient_intensity(ambientIntensity);
    }
    ImGui::Spacing();
    if (m_scene->get_skybox())
    {
        Skybox* sky = m_scene->get_skybox();
        ImGui::SeparatorText("Skybox");
        bool skyboxActive = sky->is_active();
        if (ImGui::Checkbox("Active", &skyboxActive))
        {
            sky->set_active(skyboxActive);
        }
        float skyIntensity = sky->get_intensity();
        if (ImGui::DragFloat("Sky Color Intensity", &skyIntensity, 0.05f, 0.0f, 2.0f))
        {
            sky->set_color_intensity(skyIntensity);
        }
        float skyRotation = sky->get_rotation();
        if (ImGui::DragFloat("Sky Rotation", &skyRotation, 1.0f, 0.0f, 360.0f))
        {
            sky->set_rotation(skyRotation);
        }
        const char* skyTypes[] = {"HDRi", "Procedural"};
        int         skyType    = static_cast<int>(sky->get_sky_type());
        if (ImGui::Combo("Sky Type", &skyType, skyTypes, IM_ARRAYSIZE(skyTypes)))
        {
            sky->set_sky_type(static_cast<EnviromentType>(skyType));
        }
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
        if (ImGui::ColorEdit3("Fog Color", (float*)&fogColor))
        {
            m_scene->set_fog_color(fogColor);
        }
        float fogDensity = m_scene->get_fog_intensity();
        if (ImGui::DragFloat("Density", &fogDensity, .5f, 0.0f, 1000.0f))
        {
            m_scene->set_fog_intensity(fogDensity);
        }
    }
    ImGui::SeparatorText("Accel. Structures");
    if (ImGui::Button("Update"))
    {
        m_scene->update_AS(true);
    }
    ImGui::Spacing();
    ImGui::Separator();
}
void ExplorerWidget::displayObject(Object3D* const obj, int& counter) {
    auto tpe = obj->get_type();

    ImGui::PushID(counter);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    std::string name = obj->get_name();

    if (ImGui::TreeNodeEx(
            name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth))
    {
        // node->selected = ImGui::IsItemClicked();
        if (ImGui::IsItemClicked())
        {
            if (m_selectedObject)
                m_selectedObject->set_selected(false);
            m_selectedObject = obj;
            obj->set_selected(true);
        }
    };

    ImGui::TableNextColumn();
    if (ImGui::Button(obj->is_active() ? "true" : "false"))
    {
        obj->set_active(!obj->is_active());
    }
    ImGui::PopID();

    counter++;

    ImGui::Indent();
    for (auto child : obj->get_children())
        displayObject(child, counter);
    ImGui::Unindent();
}
void ExplorerWidget::displayRendererSettings() {

    Systems::DeferredRenderer* renderer = static_cast<Systems::DeferredRenderer*>(m_renderer);
    ImGui::SeparatorText("OUTPUT");
    const char* outputTypes[] = {"LIGHTING", "ALBEDO", "NORMALS", "POSITION", "MATERIAL", "AO", "SSR"};
    static int  otype_current = static_cast<int>(renderer->get_shading_output());
    if (ImGui::Combo("Shading Output", &otype_current, outputTypes, IM_ARRAYSIZE(outputTypes)))
    {
        switch (otype_current)
        {
        case 0:
            renderer->set_shading_output(Render::OutputBuffer::LIGHTING);
            break;
        case 1:
            renderer->set_shading_output(Render::OutputBuffer::ALBEDO);
            break;
        case 2:
            renderer->set_shading_output(Render::OutputBuffer::NORMAL);
            break;
        case 3:
            renderer->set_shading_output(Render::OutputBuffer::POSITION);
            break;
        case 4:
            renderer->set_shading_output(Render::OutputBuffer::MATERIAL);
            break;
        case 5:
            renderer->set_shading_output(Render::OutputBuffer::SSAO);
            break;
        }
    }
    ImGui::SeparatorText("SCREEN SYNC");

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(renderer->get_settings().screenSync);
    if (ImGui::Combo("Sync Type", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        Render::Settings settings = renderer->get_settings();
        switch (sync_current)
        {
        case 0:
            settings.screenSync = SyncType::NONE;
            break;
        case 1:
            settings.screenSync = SyncType::MAILBOX;
            break;
        case 2:
            settings.screenSync = SyncType::VERTICAL;
            break;
        }
        renderer->set_settings(settings);
    }
    ImGui::SeparatorText("CLEARING");

    glm::vec3 clearColor = renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        Render::Settings settings = renderer->get_settings();
        settings.clearColor                = glm::vec4(clearColor, 1.0f);
        renderer->set_settings(settings);
    }

    ImGui::SeparatorText("GI");

    Render::VXGI settings_VXGI = renderer->get_VXGI_settings();
    bool       VXGIenabled   = (bool)settings_VXGI.enabled;
    if (ImGui::Checkbox("Enable VXGI", &VXGIenabled))
    {
        settings_VXGI.enabled = VXGIenabled;
        renderer->set_VXGI_settings(settings_VXGI);
    }
    if (settings_VXGI.enabled)
    {
        const char* updatMode[] = {"64", "128", "256", "512"};
        int         updateMode_current;
        switch (settings_VXGI.resolution)
        {
        case 64:
            updateMode_current = 0;
            break;
        case 128:
            updateMode_current = 1;
            break;
        case 256:
            updateMode_current = 2;
            break;
        case 512:
            updateMode_current = 3;
            break;
        }

        if (ImGui::Combo("Voxel Resolution", &updateMode_current, updatMode, IM_ARRAYSIZE(updatMode)))
        {
            switch (updateMode_current)
            {
            case 0:
                settings_VXGI.resolution = 64;
                break;
            case 1:
                settings_VXGI.resolution = 128;
                break;
            case 2:
                settings_VXGI.resolution = 256;
                break;
            case 3:
                settings_VXGI.resolution = 512;
                break;
            }
            renderer->set_VXGI_settings(settings_VXGI);
        }

        if (ImGui::DragFloat("GI Intensity", &settings_VXGI.strength, 0.1f, 0.0f, 5.0f))
        {
            renderer->set_VXGI_settings(settings_VXGI);
        }
        if (ImGui::DragFloat("Cone Offset", &settings_VXGI.offset, 0.1f, 0.0f, 10.0f))
        {
            renderer->set_VXGI_settings(settings_VXGI);
        }
        if (ImGui::DragFloat("Max Cone Distance", &settings_VXGI.maxDistance, 0.1f, 0.0f, 1000.0f))
        {
            renderer->set_VXGI_settings(settings_VXGI);
        }
        if (ImGui::DragFloat("Diffuse Cone Spread", &settings_VXGI.diffuseConeSpread, 0.01f, 0.0f, 2.0f))
        {
            renderer->set_VXGI_settings(settings_VXGI);
        }
    }

    ImGui::SeparatorText("SHADOWS");

    const char* res[] = {"VERY LOW", "LOW", "MID", "HIGH", "ULTRA"};

    int res_current;

    if (ImGui::Combo("Shadow Quality", &res_current, res, IM_ARRAYSIZE(res)))
    {
        auto settings = renderer->get_settings();
        switch (res_current)
        {
        case 0:
            settings.shadowQuality = ShadowResolution::VERY_LOW;
            break;
        case 1:
            settings.shadowQuality = ShadowResolution::LOW;
            break;
        case 2:
            settings.shadowQuality = ShadowResolution::MEDIUM;
            break;
        case 3:
            settings.shadowQuality = ShadowResolution::HIGH;
            break;
        case 4:
            settings.shadowQuality = ShadowResolution::ULTRA;
            break;
        }
        renderer->set_settings(settings);
    }
    ImGui::SeparatorText("BLOOM");

    float bloomIntensity = renderer->get_bloom_strength();

    if (ImGui::DragFloat("Bloom Intensity", &bloomIntensity, 0.01f, 0.0f, 0.5f))
    {
        renderer->set_bloom_strength(bloomIntensity);
    }
#pragma region AO
    ImGui::SeparatorText("AMBIENT OCC.");

    Render::AO settings_SSAO = renderer->get_SSAO_settings();
    bool     SSAOEnabled   = (bool)settings_SSAO.enabled;
    if (ImGui::Checkbox("Enable AO", &SSAOEnabled))
    {
        settings_SSAO.enabled = SSAOEnabled;
        renderer->set_SSAO_settings(settings_SSAO);
    }
    if (SSAOEnabled)
    {
        int         SSAOsamples  = (int)settings_SSAO.samples;
        const char* ssaoType[]   = {"SSAO", "RTAO", "VXAO"};
        int         ssao_current = (int)settings_SSAO.type;
        if (ImGui::Combo("AO Type", &ssao_current, ssaoType, IM_ARRAYSIZE(ssaoType)))
        {

            switch (ssao_current)
            {
            case 0:
                settings_SSAO.type = Render::AOType::SSAO;
                break;
            case 1:
                settings_SSAO.type = Render::AOType::RTAO;
                break;
            case 2:
                settings_SSAO.type = Render::AOType::VXAO;
                break;
            }
            renderer->set_SSAO_settings(settings_SSAO);
        }
        if (settings_SSAO.type != Render::AOType::VXAO)
        {
            if (ImGui::DragInt("SSAO Samples", &SSAOsamples, 1, 1, 64))
            {
                settings_SSAO.samples = SSAOsamples;
                renderer->set_SSAO_settings(settings_SSAO);
            }
            if (ImGui::DragFloat("SSAO Radius", &settings_SSAO.radius, 0.01f, 0.0f, 10.0f))
            {
                renderer->set_SSAO_settings(settings_SSAO);
            }
            if (ImGui::DragFloat("SSAO Bias", &settings_SSAO.bias, 0.01f, 0.0f, 10.0f))
            {
                renderer->set_SSAO_settings(settings_SSAO);
            }
            if (ImGui::DragFloat("SSAO Blur Radius", &settings_SSAO.blurRadius, 0.01f, 0.0f, 10.0f))
            {
                renderer->set_SSAO_settings(settings_SSAO);
            }
            // if (ImGui::DragFloat("SSAO Blur Sigma", &settings_SSAO.blurSigmaA, 0.01f, 0.0f, 20.0f))
            // {
            //     renderer->set_SSAO_settings(settings_SSAO);
            // }
            // if (ImGui::DragFloat("SSAO Blur Sigma B", &settings_SSAO.blurSigmaB, 0.01f, 0.0f, 10.0f))
            // {
            //     renderer->set_SSAO_settings(settings_SSAO);
            // }
        }
    }
#pragma region SSR
    ImGui::SeparatorText("SSR");

    Render::SSR settings_SSR = renderer->get_SSR_settings();
    bool      ssrEnabled   = (bool)settings_SSR.enabled;
    if (ImGui::Checkbox("Enable SSR", &ssrEnabled))
    {
        settings_SSR.enabled = ssrEnabled;
        renderer->set_SSR_settings(settings_SSR);
    }
    if (settings_SSR.enabled)
    {
        int ssrMaxSteps    = (int)settings_SSR.maxSteps;
        int ssrRefineSteps = (int)settings_SSR.binaryRefinementSteps;
        if (ImGui::DragInt("SSR Steps", &ssrMaxSteps, 1, 1, 254))
        {
            settings_SSR.maxSteps = ssrMaxSteps;
            renderer->set_SSR_settings(settings_SSR);
        }
        if (ImGui::DragFloat("SSR Stride", &settings_SSR.stride, 0.01f, 0.0f, 10.0f))
        {
            renderer->set_SSR_settings(settings_SSR);
        }
        if (ImGui::DragFloat("SSR Thickness", &settings_SSR.thickness, 0.01f, 0.0f, 5.0f))
        {
            renderer->set_SSR_settings(settings_SSR);
        }
        if (ImGui::DragInt("SSR Refinement Steps", &ssrRefineSteps, 1, 1, 10))
        {
            settings_SSR.binaryRefinementSteps = ssrRefineSteps;
            renderer->set_SSR_settings(settings_SSR);
        }
    }
#pragma region TONEMAP
    ImGui::SeparatorText("TONEMAPPING");

    float exposure = renderer->get_exposure();
    if (ImGui::DragFloat("Exposure", &exposure, 0.01f, 0.0f, 5.0f))
    {
        renderer->set_exposure(exposure);
    }
    int         tonemapType = (int)renderer->get_tonemapping_type();
    const char* tonemaps[]  = {"FILMIC", "REINDHART", "ACES"};
    if (ImGui::Combo("Tonemap Type", &tonemapType, tonemaps, IM_ARRAYSIZE(tonemaps)))
    {
        renderer->set_tonemapping_type(static_cast<Render::TonemappingType>(tonemapType));
    }
}
void ExplorerWidget::displaySkySettings() {
    if (!m_scene->get_skybox())
    {
        ImGui::Text("NO SKYBOX...");
        return;
    }

    Skybox*     sky         = m_scene->get_skybox();
    SkySettings skySettings = sky->get_sky_settings();

    // Sun Elevation (Degs)
    if (ImGui::DragFloat("Sun Elevation (Degs)", &skySettings.sunElevationDeg, 1.0f, 0.0f, 90.0f))
    {
        sky->set_sky_settings(skySettings);
    }

    // Month
    int skyMonth = static_cast<int>(skySettings.month);
    if (ImGui::DragInt("Month", &skyMonth, 1, 0, 11))
    {
        skySettings.month = skyMonth;
        sky->set_sky_settings(skySettings);
    }

    // Altitude (Kms)
    if (ImGui::DragFloat("Altitude (Kms)", &skySettings.altitude, 0.1f, 0.0f, 100.0f))
    {
        sky->set_sky_settings(skySettings);
    }

    // Aerosol Type
    const char* aerosolTypes[] = {
        "BACKGROUND", "DESERT DUST", "MARITIME CLEAN", "MARITIME MINERAL", "POLAR ANTARCTIC", "POLAR ARCTIC", "REMOTE CONTINENTAL", "RURAL", "URBAN"};
    static int aerosol_current = static_cast<int>(skySettings.aerosol);
    if (ImGui::Combo("Aerosol Type", &aerosol_current, aerosolTypes, IM_ARRAYSIZE(aerosolTypes)))
    {
        skySettings.aerosol = static_cast<SkyAerosolType>(aerosol_current);
        sky->set_sky_settings(skySettings);
    }
    if (ImGui::DragFloat("Aerosol Turbidity", &skySettings.aerosolTurbidity, 0.1f, 0.0f, 25.0f))
    {
        sky->set_sky_settings(skySettings);
    }

    // Ground Albedo (Vec4)
    if (ImGui::ColorEdit4("Ground Albedo", &skySettings.groundAlbedo.x))
    {
        sky->set_sky_settings(skySettings);
    }

    // Resolution (Combo)
    const char* resolutions[]      = {"512", "1024", "2048", "4096"};
    static int  resolution_current = static_cast<int>(log2(skySettings.resolution) - 8); // Assuming 1024 is the default base
    if (ImGui::Combo("Resolution", &resolution_current, resolutions, IM_ARRAYSIZE(resolutions)))
    {
        skySettings.resolution = static_cast<uint32_t>(pow(2, resolution_current + 8)); // Convert to actual resolution value
        sky->set_sky_settings(skySettings);
    }
    if (ImGui::Checkbox("Use for IBL", &skySettings.useForIBL))
        sky->set_sky_settings(skySettings);

    // Update Type (Combo)
    const char* updateTypes[]      = {"PER FRAME", "ON DEMAND"};
    static int  updateType_current = static_cast<int>(skySettings.updateType);
    if (ImGui::Combo("Update Type", &updateType_current, updateTypes, IM_ARRAYSIZE(updateTypes)))
    {
        skySettings.updateType = static_cast<UpdateType>(updateType_current);
        sky->set_sky_settings(skySettings);
    }
}

void Profiler::render() {
    ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}
void Space::render() {
    ImGui::Spacing();
}
void Separator::render() {
    !m_text ? ImGui::Separator() : ImGui::SeparatorText(m_text);
}
#pragma region OBJECT EXPLORER

void ObjectExplorerWidget::render() {
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

    float position[3] = {m_object->get_position().x, m_object->get_position().y, m_object->get_position().z};
    if (ImGui::DragFloat3("Position", position, 0.1f))
    {
        m_object->set_position(Vec3(position[0], position[1], position[2]));
    };
    float rotation[3] = {(m_object->get_rotation().x), (m_object->get_rotation().y), (m_object->get_rotation().z)};
    if (ImGui::DragFloat3("Rotation", rotation, 0.1f))
    {
        m_object->set_rotation(Vec3(rotation[0], rotation[1], rotation[2]));
    };
    float scale[3] = {m_object->get_scale().x, m_object->get_scale().y, m_object->get_scale().z};
    if (ImGui::DragFloat3("Scale", scale, 0.1f))
    {
        m_object->set_scale(Vec3(scale[0], scale[1], scale[2]));
    };
    if (m_object->get_type() == ObjectType::MESH)
    {
        ImGui::SeparatorText("Mesh");
        Mesh* model = static_cast<Mesh*>(m_object);

        int faceCount   = 0;
        int vertexCount = 0;

        vertexCount += (int)get_VAO(model->get_geometry())->vertexCount;
        faceCount += (int)get_VAO(model->get_geometry())->indexCount / 3;

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
        ImGui::Text("Cast shadows");
        ImGui::TableNextColumn();
        bool shadows = model->cast_shadows();
        if (ImGui::Checkbox("", &shadows))
        {
            model->cast_shadows(shadows);
        };

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Affected by fog");
        ImGui::TableNextColumn();
        bool fog = model->affected_by_fog();
        if (ImGui::Checkbox("", &fog))
        {
            model->affected_by_fog(fog);
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
                const char* faceVisibility[]      = {"FRONT", "BACK"};
                static int  currentFaceVisibility = model->get_material(i)->get_parameters().culling;
                if (ImGui::Combo("Face", &currentFaceVisibility, faceVisibility, IM_ARRAYSIZE(faceVisibility)))
                {
                    MaterialSettings parameters = model->get_material(i)->get_parameters();
                    switch (currentFaceVisibility)
                    {
                    case 0:
                        model->get_material(i)->set_culling_type(FRONT_CULLING);
                        break;
                    case 1:
                        model->get_material(i)->set_culling_type(BACK_CULLING);
                        break;
                    }
                };
            }
            bool alphatest = model->get_material(i)->get_parameters().alphaTest;
            if (ImGui::Checkbox("Alpha Test", &alphatest))
            {
                model->get_material(i)->enable_alpha_test(alphatest);
            }
            bool blending = model->get_material(i)->get_parameters().blending;
            if (ImGui::Checkbox("Blending", &blending))
            {
                model->get_material(i)->enable_blending(blending);
            }
            // ImGui::Text("BLENDING AND OPACITY UNSUPORTED FOR NOW");
            // const char *blendingType[] = {"NORMAL", "ADDITIVE", "CUSTOM"};
            // static int currentBlending = model->get_material(i)->get_parameters().blending;
            // if (ImGui::Combo("Blending function", &currentBlending, blending, IM_ARRAYSIZE(blending)))
            // {
            //     switch (currentBlending)
            //     {
            //     case 0:
            //         // model->getMaterialReference(i)->setBlending(BlendingType::NORMAL);
            //         break;
            //     case 1:
            //         // model->getMaterialReference(i)->setBlending(BlendingType::ADDITIVE);
            //         break;
            //     case 2:
            //         // model->getMaterialReference(i)->setBlending(BlendingType::CUSTOM);
            //         break;
            //     }
            // };
            ImGui::Separator();
            ImVec2 texSize = {160, 160};
            if (model->get_material(i)->get_shaderpass_ID() == "physical")
            {
                PhysicalMaterial* mat    = static_cast<PhysicalMaterial*>(model->get_material(i));
                Vec3              albedo = mat->get_albedo();
                if (ImGui::ColorEdit3("Albedo", (float*)&albedo))
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
                    ImGui::Image(get_image(mat->get_albedo_texture())->GUIReadHandle, texSize);
                }
                float opacity = mat->get_opacity();
                if (ImGui::DragFloat("Opacity", &opacity, 0.05f, 0.0f, 1.0f))
                {
                    mat->set_opacity(opacity);
                }
                if (mat->get_albedo_texture())
                {
                    float weight = mat->get_opacity_weight();
                    if (ImGui::DragFloat("Op. Text Weight", &weight, 0.05f, 0.0f, 1.0f))
                    {
                        mat->set_opacity_weight(weight);
                    }
                }
                bool refl = mat->reflective();
                if (ImGui::Checkbox("Is Refelctive", &refl))
                {
                    mat->reflective(refl);
                }
                ImGui::Spacing();
                if (mat->get_normal_texture())
                {
                    ImGui::Image(get_image(mat->get_normal_texture())->GUIReadHandle, texSize);
                    bool useNormal = mat->use_normal_texture();
                    if (ImGui::Checkbox("Use Normal Texture", &useNormal))
                    {
                        mat->use_normal_texture(useNormal);
                    }
                }
                ImGui::Spacing();

                float metallic  = mat->get_metalness();
                float roughness = mat->get_roughness();
                float ao        = mat->get_occlusion();
                if (mat->get_mask_type() == MaskType::NO_MASK)
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
                        ImGui::Image(get_image(mat->get_metallic_texture())->GUIReadHandle, texSize);
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
                        ImGui::Image(get_image(mat->get_roughness_texture())->GUIReadHandle, texSize);
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
                        ImGui::Image(get_image(mat->get_occlusion_texture())->GUIReadHandle, texSize);
                    }
                } else
                {
                    switch (mat->get_mask_type())
                    {
                    case MaskType::UNREAL_ENGINE:
                        break;
                        ImGui::Text("Unreal Mask");
                    case MaskType::UNITY_HDRP:
                        ImGui::Text("Unity HDRP Mask");
                        break;
                    case MaskType::UNITY_URP:
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
                        ImGui::Image(get_image(mat->get_mask_texture())->GUIReadHandle, texSize);
                    }
                }

                Vec3 emission = mat->get_emissive_color();
                if (ImGui::ColorEdit3("Emission", (float*)&emission))
                {
                    mat->set_emissive_color(emission);
                };
                float emissionIntensity = mat->get_emission_intensity();
                if (ImGui::DragFloat("Emission Intensity", &emissionIntensity, 0.5f, 0.0f, 50.0f))
                {
                    mat->set_emission_intensity(emissionIntensity);
                }
                if (mat->get_emissive_texture())
                {
                    float emissionW = mat->get_emissive_weight();
                    if (ImGui::DragFloat("Emission Text Weight", &emissionW, 0.05f, 0.0f, 1.0f))
                    {
                        mat->set_emissive_weight(emissionW);
                    }
                    ImGui::Image(get_image(mat->get_emissive_texture())->GUIReadHandle, texSize);
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
            if (model->get_material(i)->get_shaderpass_ID() == "hairstr" || model->get_material(i)->get_shaderpass_ID() == "hairstr2")
            {
                HairStrandMaterial* mat = static_cast<HairStrandMaterial*>(model->get_material(i));
                // ImGui UI code
                Vec3 baseColor = mat->get_base_color();
                if (ImGui::ColorEdit3("Base Color (RGBA)", (float*)&baseColor))
                {
                    mat->set_base_color(baseColor);
                }

                float thickness = mat->get_thickness();
                if (ImGui::DragFloat("Thickness", &thickness, 0.0001f, 0.0f, 1.0f))
                {
                    mat->set_thickness(thickness);
                }

                bool r = mat->get_R();
                if (ImGui::Checkbox("Reflection (R)", &r))
                {
                    mat->set_R(r);
                }
                float rPower = mat->get_Rpower();
                if (ImGui::DragFloat("Reflection Power (R)", &rPower, 0.1f, 0.0f, 10.0f))
                {
                    mat->set_Rpower(rPower); // Update reflection power
                }

                bool tt = mat->get_TT();
                if (ImGui::Checkbox("Transmitance (TT)", &tt))
                {
                    mat->set_TT(tt); // Update transmittance
                }

                float ttPower = mat->get_TTpower();
                if (ImGui::DragFloat("Transmitance Power (TT)", &ttPower, 0.1f, 0.0f, 10.0f))
                {
                    mat->set_TTpower(ttPower); // Update transmittance power
                }

                bool trt = mat->get_TRT();
                if (ImGui::Checkbox("Second Reflection (TRT)", &trt))
                {
                    mat->set_TRT(trt); // Update second reflection
                }

                float trtPower = mat->get_TRTpower();
                if (ImGui::DragFloat("Second Reflection Power (TRT)", &trtPower, 0.1f, 0.0f, 20.0f))
                {
                    mat->set_TRTpower(trtPower); // Update second reflection power
                }

                float roughness = mat->get_roughness();
                if (ImGui::DragFloat("Roughness", &roughness, 0.01f, 0.0f, 1.0f))
                {
                    mat->set_roughness(roughness); // Update roughness
                }

                float scatter = mat->get_scatter();
                if (ImGui::DragFloat("Density", &scatter, 1.0f, 0.0f, 100.0f))
                {
                    mat->set_scatter(scatter); // Update scatter
                }

                float shift = mat->get_shift();
                if (ImGui::DragFloat("Shift (degrees)", &shift, 0.1f, 1.0f, 20.0f))
                {
                    mat->set_shift(shift); // Update shift
                }

                float ior = mat->get_ior();
                if (ImGui::DragFloat("Index of Refraction (IOR)", &ior, 0.01f, 1.0f, 2.0f))
                {
                    mat->set_ior(ior); // Update index of refraction
                }

                bool useScatter = mat->get_useScatter();
                if (ImGui::Checkbox("Local Scatter", &useScatter))
                {
                    mat->set_useScatter(useScatter); // Update use scatter
                }

                bool coloredScatter = mat->get_coloredScatter();
                if (ImGui::Checkbox("Global Scatter", &coloredScatter))
                {
                    mat->set_coloredScatter(coloredScatter); // Update colored scatter
                }
                bool glints = mat->get_glints();
                if (ImGui::Checkbox("Glints", &glints))
                {
                    mat->set_glints(glints); // Update glints
                }
                bool occlusion = mat->get_occlusion();
                if (ImGui::Checkbox("Occlusion", &occlusion))
                {
                    mat->set_occlusion(occlusion); // Update occlusion
                }
            }

            if (model->get_material(i)->get_shaderpass_ID() == "unlit")
            {
                UnlitMaterial* mat    = static_cast<UnlitMaterial*>(model->get_material(i));
                Vec3           albedo = mat->get_color();
                if (ImGui::ColorEdit3("Color", (float*)&albedo))
                {
                    mat->set_color(Vec4{albedo, mat->get_color().a});
                };

                float opacity = mat->get_color().a;
                if (ImGui::DragFloat("Opacity", &opacity, 0.05f, 0.0f, 1.0f))
                {
                    mat->set_color(Vec4{Vec3(mat->get_color()), opacity});
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
            }
            if (model->get_material(i)->get_shaderpass_ID() == "phong")
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

        Light* light = static_cast<Light*>(m_object);

        if (light->get_light_type() == LightType::DIRECTIONAL)
        {
            DirectionalLight* dirL     = static_cast<DirectionalLight*>(light);
            float             dir[3]   = {dirL->get_direction().x, dirL->get_direction().y, dirL->get_direction().z};
            bool              useAsSun = dirL->use_as_sun();
            if (ImGui::Checkbox("Use As Sun", &useAsSun))
            {
                dirL->use_as_sun(useAsSun);
            };
            if (!useAsSun)
                if (ImGui::DragFloat3("Direction", dir, 0.05f, -1.0f, 1.0f))
                {
                    dirL->set_direction(Vec3(dir[0], dir[1], dir[2]));
                };
        }

        float intensity = light->get_intensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.005f, 0.0f, 10.0f))
            light->set_intensity(intensity);
        Vec3 color = light->get_color();
        if (ImGui::ColorEdit3("Color", (float*)&color))
        {
            light->set_color(color);
        };

        if (light->get_light_type() == LightType::POINT)
        {
            float att = static_cast<PointLight*>(light)->get_area_of_effect();
            if (ImGui::DragFloat("Area of Influence", &att, 0.005f, 0.0f, 50.0f))
                static_cast<PointLight*>(light)->set_area_of_effect(att);
        }
        bool castShadows = light->get_cast_shadows();

        if (ImGui::Checkbox("Cast Shadows", &castShadows))
        {
            light->set_cast_shadows(castShadows);
        };
        if (castShadows)
        {
            ImGui::Spacing();
            const char* shadowTypes[] = {"CLASSIC", "VSM", "RAYTRACED"};
            int         currentShadow = static_cast<int>(light->get_shadow_type());
            if (ImGui::Combo("Shadow Type", &currentShadow, shadowTypes, IM_ARRAYSIZE(shadowTypes)))
            {
                switch (currentShadow)
                {
                case 0:
                    light->set_shadow_type(ShadowType::BASIC_SHADOW);
                    break;
                case 1:
                    light->set_shadow_type(ShadowType::VSM_SHADOW);
                    break;
                case 2:
                    light->set_shadow_type(ShadowType::RAYTRACED_SHADOW);
                    break;
                }
            }
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Advanced Shadow Settings:");
            if (currentShadow == 0)
            {
                float shadowFov = light->get_shadow_fov();
                if (ImGui::DragFloat("FOV", &shadowFov, 1.0f, 0.0f, 160.0f))
                    light->set_shadow_fov(shadowFov);
                float position[3] = {light->get_shadow_target().x, light->get_shadow_target().y, light->get_shadow_target().z};
                if (ImGui::DragFloat3("Target", position, 0.1f))
                {
                    light->set_shadow_target(Vec3(position[0], position[1], position[2]));
                };
                float shadowNear = light->get_shadow_near();
                if (ImGui::DragFloat("Near Plane", &shadowNear, 0.005f, 0.0f, 10.0f))
                    light->set_shadow_near(shadowNear);
                float shadowFar = light->get_shadow_far();
                if (ImGui::DragFloat("Far Plane", &shadowFar, 1.0f, 10.0f, 1000.0f))
                    light->set_shadow_far(shadowFar);
                float bias = light->get_shadow_bias();
                if (ImGui::DragFloat("Bias", &bias, 0.0001f, 0.0f, 1.0f))
                    light->set_shadow_bias(bias);
                int kernel = light->get_shadow_softness();
                if (ImGui::DragInt("Softness", &kernel, 2, 3, 15))
                    light->set_shadow_softness(kernel);

                float kernelRad = light->get_shadow_kernel_radius();
                if (ImGui::DragFloat("Softness Magnifier", &kernelRad, 0.1f, 1.0f, 100.0f))
                    light->set_shadow_kernel_radius(kernelRad);
            }
            if (currentShadow == 1)
            {
                float shadowFov = light->get_shadow_fov();
                if (ImGui::DragFloat("FOV", &shadowFov, 1.0f, 0.0f, 160.0f))
                    light->set_shadow_fov(shadowFov);
                float position[3] = {light->get_shadow_target().x, light->get_shadow_target().y, light->get_shadow_target().z};
                if (ImGui::DragFloat3("Target", position, 0.1f))
                {
                    light->set_shadow_target(Vec3(position[0], position[1], position[2]));
                };
                float bleeding = light->get_shadow_bleeding();
                if (ImGui::DragFloat("Bleeding", &bleeding, 0.001f, 0.0f, 1.0f))
                    light->set_shadow_bleeding(bleeding);
                int kernel = light->get_shadow_softness();
                if (ImGui::DragInt("Softness", &kernel, 2, 3, 15))
                    light->set_shadow_softness(kernel);

                float kernelRad = light->get_shadow_kernel_radius();
                if (ImGui::DragFloat("Softness Magnifier", &kernelRad, 0.1f, 1.0f, 100.0f))
                    light->set_shadow_kernel_radius(kernelRad);
            }
            if (currentShadow == 2)
            {
                float area = light->get_area();
                if (ImGui::DragFloat("Area", &area, 0.05f, 0.0f, 5.0f))
                    light->set_area(area);
                int samples = light->get_shadow_ray_samples();
                if (ImGui::DragInt("Ray samples", &samples, 1, 0, 16))
                    light->set_shadow_ray_samples(samples);
            }
        }
    }
    if (m_object->get_type() == ObjectType::CAMERA)
    {
        ImGui::SeparatorText("Camera");

        Camera* cam     = static_cast<Camera*>(m_object);
        float   _far    = cam->get_far();
        float   _near   = cam->get_near();
        float   fov     = cam->get_field_of_view();
        bool    culling = cam->get_frustrum_culling();

        if (ImGui::DragFloat("Near", &_near, 0.05f, 0.0f, 10.0))
            cam->set_near(_near);
        if (ImGui::DragFloat("Far", &_far, 0.1f, 0.0f, 9999.0f))
            cam->set_far(_far);
        if (ImGui::DragFloat("Field of view", &fov, 0.1f, 0.0f, 160.0f))
            cam->set_field_of_view(fov);
        if (ImGui::Checkbox("Frustum Culling", &culling))
            cam->enable_frustrum_culling(culling);
    }
}
void ControllerWidget::render() {
    if (!m_controller)
        return;
    ImGui::SeparatorText("Controller");

    float speed = m_controller->get_speed();
    if (ImGui::DragFloat("Speed", &speed, 0.05f, 0.0f, 100.0))
        m_controller->set_speed(speed);
    float sensitivity = m_controller->get_mouse_sensitivity();
    if (ImGui::DragFloat("Sensitivity", &sensitivity, 0.05f, 0.0f, 100.0))
        m_controller->set_mouse_sensitivity(sensitivity);
    ImGui::Spacing();
    const char* controllerType[] = {
        "WASD",
        "ORBITAL",
    };
    int currentType = static_cast<int>(m_controller->get_type());
    if (ImGui::Combo("Type", &currentType, controllerType, IM_ARRAYSIZE(controllerType)))
    {
        switch (currentType)
        {
        case 0:
            m_controller->set_type(ControllerMovementType::WASD);
            break;
        case 1:
            m_controller->set_type(ControllerMovementType::ORBITAL);
            break;
        }
    }

    // float position[3] = {m_object->get_position().x, m_object->get_position().y, m_object->get_position().z};
    // if (ImGui::DragFloat3("Position", position, 0.1f))
    // {
    //     m_object->set_position(Vec3(position[0], position[1], position[2]));
    // };
}
void RendererSettingsWidget::render() {

#pragma region SYNC
    ImGui::SeparatorText("Renderer Settings");

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Sync Type", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        Render::Settings settings = m_renderer->get_settings();
        switch (sync_current)
        {
        case 0:
            settings.screenSync = SyncType::NONE;
            break;
        case 1:
            settings.screenSync = SyncType::MAILBOX;
            break;
        case 2:
            settings.screenSync = SyncType::VERTICAL;
            break;
        }
        m_renderer->set_settings(settings);
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        Render::Settings settings = m_renderer->get_settings();
        settings.clearColor                = glm::vec4(clearColor, 1.0f);
        m_renderer->set_settings(settings);
    }
}
} // namespace Tools
void Tools::ForwardRendererWidget::render() {
#pragma region SYNC
    ImGui::SeparatorText("Renderer Settings");

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Sync Type", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        Render::Settings settings = m_renderer->get_settings();
        switch (sync_current)
        {
        case 0:
            settings.screenSync = SyncType::NONE;
            break;
        case 1:
            settings.screenSync = SyncType::MAILBOX;
            break;
        case 2:
            settings.screenSync = SyncType::VERTICAL;
            break;
        }
        m_renderer->set_settings(settings);
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        Render::Settings settings = m_renderer->get_settings();
        settings.clearColor                = glm::vec4(clearColor, 1.0f);
        m_renderer->set_settings(settings);
    }

#pragma region SHADOWS
    const char* res[] = {"VERY LOW", "LOW", "MID", "HIGH", "ULTRA"};

    int res_current;

    if (ImGui::Combo("Shadow Quality", &res_current, res, IM_ARRAYSIZE(res)))
    {
        auto settings = m_renderer->get_settings();
        switch (res_current)
        {
        case 0:
            settings.shadowQuality = ShadowResolution::VERY_LOW;
            break;
        case 1:
            settings.shadowQuality = ShadowResolution::LOW;
            break;
        case 2:
            settings.shadowQuality = ShadowResolution::MEDIUM;
            break;
        case 3:
            settings.shadowQuality = ShadowResolution::HIGH;
            break;
        case 4:
            settings.shadowQuality = ShadowResolution::ULTRA;
            break;
        }
        m_renderer->set_settings(settings);
    }
    ImGui::Separator();

#pragma region BLOOM
    float bloomIntensity = m_renderer->get_bloom_strength();
    if (ImGui::DragFloat("Bloom Intensity", &bloomIntensity, 0.01f, 0.0f, 0.5f))
    {
        m_renderer->set_bloom_strength(bloomIntensity);
    }
}
// namespace Tools
void Tools::DeferredRendererWidget::render() {
#pragma region OUTPUT
    ImGui::SeparatorText("Renderer Settings");

    const char* outputTypes[] = {"LIGHTING", "ALBEDO", "NORMALS", "POSITION", "MATERIAL", "AO", "SSR"};
    static int  otype_current = static_cast<int>(m_renderer->get_shading_output());
    if (ImGui::Combo("Shading Output", &otype_current, outputTypes, IM_ARRAYSIZE(outputTypes)))
    {
        switch (otype_current)
        {
        case 0:
            m_renderer->set_shading_output(Render::OutputBuffer::LIGHTING);
            break;
        case 1:
            m_renderer->set_shading_output(Render::OutputBuffer::ALBEDO);
            break;
        case 2:
            m_renderer->set_shading_output(Render::OutputBuffer::NORMAL);
            break;
        case 3:
            m_renderer->set_shading_output(Render::OutputBuffer::POSITION);
            break;
        case 4:
            m_renderer->set_shading_output(Render::OutputBuffer::MATERIAL);
            break;
        case 5:
            m_renderer->set_shading_output(Render::OutputBuffer::SSAO);
            break;
        }
    }

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Sync Type", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        Render::Settings settings = m_renderer->get_settings();
        switch (sync_current)
        {
        case 0:
            settings.screenSync = SyncType::NONE;
            break;
        case 1:
            settings.screenSync = SyncType::MAILBOX;
            break;
        case 2:
            settings.screenSync = SyncType::VERTICAL;
            break;
        }
        m_renderer->set_settings(settings);
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        Render::Settings settings = m_renderer->get_settings();
        settings.clearColor                = glm::vec4(clearColor, 1.0f);
        m_renderer->set_settings(settings);
    }

    ImGui::Separator();
    Render::VXGI settings_VXGI = m_renderer->get_VXGI_settings();
    bool       VXGIenabled   = (bool)settings_VXGI.enabled;
    if (ImGui::Checkbox("Enable VXGI", &VXGIenabled))
    {
        settings_VXGI.enabled = VXGIenabled;
        m_renderer->set_VXGI_settings(settings_VXGI);
    }
    if (settings_VXGI.enabled)
    {
        const char* updatMode[] = {"64", "128", "256", "512"};
        int         updateMode_current;
        switch (settings_VXGI.resolution)
        {
        case 64:
            updateMode_current = 0;
            break;
        case 128:
            updateMode_current = 1;
            break;
        case 256:
            updateMode_current = 2;
            break;
        case 512:
            updateMode_current = 3;
            break;
        }

        if (ImGui::Combo("Voxel Resolution", &updateMode_current, updatMode, IM_ARRAYSIZE(updatMode)))
        {
            switch (updateMode_current)
            {
            case 0:
                settings_VXGI.resolution = 64;
                break;
            case 1:
                settings_VXGI.resolution = 128;
                break;
            case 2:
                settings_VXGI.resolution = 256;
                break;
            case 3:
                settings_VXGI.resolution = 512;
                break;
            }
            m_renderer->set_VXGI_settings(settings_VXGI);
        }

        if (ImGui::DragFloat("GI Intensity", &settings_VXGI.strength, 0.1f, 0.0f, 5.0f))
        {
            m_renderer->set_VXGI_settings(settings_VXGI);
        }
        if (ImGui::DragFloat("Cone Offset", &settings_VXGI.offset, 0.1f, 0.0f, 10.0f))
        {
            m_renderer->set_VXGI_settings(settings_VXGI);
        }
        if (ImGui::DragFloat("Max Cone Distance", &settings_VXGI.maxDistance, 0.1f, 0.0f, 1000.0f))
        {
            m_renderer->set_VXGI_settings(settings_VXGI);
        }
        if (ImGui::DragFloat("Diffuse Cone Spread", &settings_VXGI.diffuseConeSpread, 0.01f, 0.0f, 2.0f))
        {
            m_renderer->set_VXGI_settings(settings_VXGI);
        }
    }

    ImGui::Separator();
    const char* res[] = {"VERY LOW", "LOW", "MID", "HIGH", "ULTRA"};

    int res_current;

    if (ImGui::Combo("Shadow Quality", &res_current, res, IM_ARRAYSIZE(res)))
    {
        auto settings = m_renderer->get_settings();
        switch (res_current)
        {
        case 0:
            settings.shadowQuality = ShadowResolution::VERY_LOW;
            break;
        case 1:
            settings.shadowQuality = ShadowResolution::LOW;
            break;
        case 2:
            settings.shadowQuality = ShadowResolution::MEDIUM;
            break;
        case 3:
            settings.shadowQuality = ShadowResolution::HIGH;
            break;
        case 4:
            settings.shadowQuality = ShadowResolution::ULTRA;
            break;
        }
        m_renderer->set_settings(settings);
    }
    ImGui::Separator();

    float bloomIntensity = m_renderer->get_bloom_strength();

    if (ImGui::DragFloat("Bloom Intensity", &bloomIntensity, 0.01f, 0.0f, 0.5f))
    {
        m_renderer->set_bloom_strength(bloomIntensity);
    }
#pragma region AO
    ImGui::Separator();
    Render::AO settings_SSAO = m_renderer->get_SSAO_settings();
    bool     SSAOEnabled   = (bool)settings_SSAO.enabled;
    if (ImGui::Checkbox("Enable AO", &SSAOEnabled))
    {
        settings_SSAO.enabled = SSAOEnabled;
        m_renderer->set_SSAO_settings(settings_SSAO);
    }
    if (SSAOEnabled)
    {
        int         SSAOsamples  = (int)settings_SSAO.samples;
        const char* ssaoType[]   = {"SSAO", "RTAO", "VXAO"};
        int         ssao_current = (int)settings_SSAO.type;
        if (ImGui::Combo("AO Type", &ssao_current, ssaoType, IM_ARRAYSIZE(ssaoType)))
        {

            switch (ssao_current)
            {
            case 0:
                settings_SSAO.type = Render::AOType::SSAO;
                break;
            case 1:
                settings_SSAO.type = Render::AOType::RTAO;
                break;
            case 2:
                settings_SSAO.type = Render::AOType::VXAO;
                break;
            }
            m_renderer->set_SSAO_settings(settings_SSAO);
        }
        if (settings_SSAO.type != Render::AOType::VXAO)
        {
            if (ImGui::DragInt("SSAO Samples", &SSAOsamples, 1, 1, 64))
            {
                settings_SSAO.samples = SSAOsamples;
                m_renderer->set_SSAO_settings(settings_SSAO);
            }
            if (ImGui::DragFloat("SSAO Radius", &settings_SSAO.radius, 0.01f, 0.0f, 10.0f))
            {
                m_renderer->set_SSAO_settings(settings_SSAO);
            }
            if (ImGui::DragFloat("SSAO Bias", &settings_SSAO.bias, 0.01f, 0.0f, 10.0f))
            {
                m_renderer->set_SSAO_settings(settings_SSAO);
            }
            if (ImGui::DragFloat("SSAO Blur Radius", &settings_SSAO.blurRadius, 0.01f, 0.0f, 10.0f))
            {
                m_renderer->set_SSAO_settings(settings_SSAO);
            }
            // if (ImGui::DragFloat("SSAO Blur Sigma", &settings_SSAO.blurSigmaA, 0.01f, 0.0f, 20.0f))
            // {
            //     m_renderer->set_SSAO_settings(settings_SSAO);
            // }
            // if (ImGui::DragFloat("SSAO Blur Sigma B", &settings_SSAO.blurSigmaB, 0.01f, 0.0f, 10.0f))
            // {
            //     m_renderer->set_SSAO_settings(settings_SSAO);
            // }
        }
    }
#pragma region SSR
    ImGui::Separator();
    Render::SSR settings_SSR = m_renderer->get_SSR_settings();
    bool      ssrEnabled   = (bool)settings_SSR.enabled;
    if (ImGui::Checkbox("Enable SSR", &ssrEnabled))
    {
        settings_SSR.enabled = ssrEnabled;
        m_renderer->set_SSR_settings(settings_SSR);
    }
    if (settings_SSR.enabled)
    {
        int ssrMaxSteps    = (int)settings_SSR.maxSteps;
        int ssrRefineSteps = (int)settings_SSR.binaryRefinementSteps;
        if (ImGui::DragInt("SSR Steps", &ssrMaxSteps, 1, 1, 254))
        {
            settings_SSR.maxSteps = ssrMaxSteps;
            m_renderer->set_SSR_settings(settings_SSR);
        }
        if (ImGui::DragFloat("SSR Stride", &settings_SSR.stride, 0.01f, 0.0f, 10.0f))
        {
            m_renderer->set_SSR_settings(settings_SSR);
        }
        if (ImGui::DragFloat("SSR Thickness", &settings_SSR.thickness, 0.01f, 0.0f, 5.0f))
        {
            m_renderer->set_SSR_settings(settings_SSR);
        }
        if (ImGui::DragInt("SSR Refinement Steps", &ssrRefineSteps, 1, 1, 10))
        {
            settings_SSR.binaryRefinementSteps = ssrRefineSteps;
            m_renderer->set_SSR_settings(settings_SSR);
        }
    }
#pragma region TONEMAP
    ImGui::Separator();
    float exposure = m_renderer->get_exposure();
    if (ImGui::DragFloat("Exposure", &exposure, 0.01f, 0.0f, 5.0f))
    {
        m_renderer->set_exposure(exposure);
    }
    int         tonemapType = (int)m_renderer->get_tonemapping_type();
    const char* tonemaps[]  = {"FILMIC", "REINDHART", "ACES"};
    if (ImGui::Combo("Tonemap Type", &tonemapType, tonemaps, IM_ARRAYSIZE(tonemaps)))
    {
        m_renderer->set_tonemapping_type(static_cast<Render::TonemappingType>(tonemapType));
    }

} // namespace Tools

VULKAN_ENGINE_NAMESPACE_END
