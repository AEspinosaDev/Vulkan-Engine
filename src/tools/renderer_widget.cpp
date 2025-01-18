#include <engine/tools/renderer_widget.h>
VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Tools {
void RendererSettingsWidget::render() {

#pragma region SYNC
    ImGui::SeparatorText("Renderer Settings");

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Screen Sync", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        switch (sync_current)
        {
        case 0:
            m_renderer->set_sync_type(SyncType::NONE);
            break;
        case 1:
            m_renderer->set_sync_type(SyncType::MAILBOX);
            break;
        case 2:
            m_renderer->set_sync_type(SyncType::VERTICAL);
            break;
        }
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        m_renderer->set_clearcolor(glm::vec4(clearColor, 1.0f));
    }
}
} // namespace Tools
void Tools::ForwardRendererWidget::render() {
#pragma region SYNC
    ImGui::SeparatorText("Renderer Settings");

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Screen Sync", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        switch (sync_current)
        {
        case 0:
            m_renderer->set_sync_type(SyncType::NONE);
            break;
        case 1:
            m_renderer->set_sync_type(SyncType::MAILBOX);
            break;
        case 2:
            m_renderer->set_sync_type(SyncType::VERTICAL);
            break;
        }
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        m_renderer->set_clearcolor(glm::vec4(clearColor, 1.0f));
    }

#pragma region SHADOWS
    const char* res[] = {"VERY LOW", "LOW", "MID", "HIGH", "ULTRA"};

    int res_current;

    if (ImGui::Combo("Shadow Quality", &res_current, res, IM_ARRAYSIZE(res)))
    {
        switch (res_current)
        {
        case 0:
            m_renderer->set_shadow_quality(ShadowResolution::VERY_LOW);
            break;
        case 1:
            m_renderer->set_shadow_quality(ShadowResolution::LOW);
            break;
        case 2:
            m_renderer->set_shadow_quality(ShadowResolution::MEDIUM);
            break;
        case 3:
            m_renderer->set_shadow_quality(ShadowResolution::HIGH);
            break;
        case 4:
            m_renderer->set_shadow_quality(ShadowResolution::ULTRA);
            break;
        }
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
            m_renderer->set_shading_output(Core::OutputBuffer::LIGHTING);
            break;
        case 1:
            m_renderer->set_shading_output(Core::OutputBuffer::ALBEDO);
            break;
        case 2:
            m_renderer->set_shading_output(Core::OutputBuffer::NORMAL);
            break;
        case 3:
            m_renderer->set_shading_output(Core::OutputBuffer::POSITION);
            break;
        case 4:
            m_renderer->set_shading_output(Core::OutputBuffer::MATERIAL);
            break;
        case 5:
            m_renderer->set_shading_output(Core::OutputBuffer::SSAO);
            break;
        }
    }

    const char* syncs[]      = {"NONE", "MAILBOX", "VSYNC"};
    static int  sync_current = static_cast<int>(m_renderer->get_settings().screenSync);
    if (ImGui::Combo("Screen Sync", &sync_current, syncs, IM_ARRAYSIZE(syncs)))
    {
        switch (sync_current)
        {
        case 0:
            m_renderer->set_sync_type(SyncType::NONE);
            break;
        case 1:
            m_renderer->set_sync_type(SyncType::MAILBOX);
            break;
        case 2:
            m_renderer->set_sync_type(SyncType::VERTICAL);
            break;
        }
    }

    glm::vec3 clearColor = m_renderer->get_settings().clearColor;
    if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
    {
        m_renderer->set_clearcolor(glm::vec4(clearColor, 1.0f));
    }

    ImGui::Separator();
    Core::VXGI settings_VXGI = m_renderer->get_VXGI_settings();
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
        switch (res_current)
        {
        case 0:
            m_renderer->set_shadow_quality(ShadowResolution::VERY_LOW);
            break;
        case 1:
            m_renderer->set_shadow_quality(ShadowResolution::LOW);
            break;
        case 2:
            m_renderer->set_shadow_quality(ShadowResolution::MEDIUM);
            break;
        case 3:
            m_renderer->set_shadow_quality(ShadowResolution::HIGH);
            break;
        case 4:
            m_renderer->set_shadow_quality(ShadowResolution::ULTRA);
            break;
        }
    }
    ImGui::Separator();

    float bloomIntensity = m_renderer->get_bloom_strength();

    if (ImGui::DragFloat("Bloom Intensity", &bloomIntensity, 0.01f, 0.0f, 0.5f))
    {
        m_renderer->set_bloom_strength(bloomIntensity);
    }
#pragma region AO
    ImGui::Separator();
    Core::AO settings_SSAO = m_renderer->get_SSAO_settings();
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
                settings_SSAO.type = Core::AOType::SSAO;
                break;
            case 1:
                settings_SSAO.type = Core::AOType::RTAO;
                break;
            case 2:
                settings_SSAO.type = Core::AOType::VXAO;
                break;
            }
            m_renderer->set_SSAO_settings(settings_SSAO);
        }
        if (settings_SSAO.type != Core::AOType::VXAO)
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
    Core::SSR settings_SSR = m_renderer->get_SSR_settings();
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
}
// namespace Tools
VULKAN_ENGINE_NAMESPACE_END