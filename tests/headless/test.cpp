#include "test.h"
#include <filesystem>

void Application::init(Systems::RendererSettings settings) {

    m_renderer                      = std::make_shared<Systems::DeferredRenderer>();

    m_renderer->set_settings(settings);

    setup();
    m_renderer->init();
}

void Application::run(int argc, char* argv[]) {

    Systems::RendererSettings settings{};
    settings.bufferingType         = BufferingType::DOUBLE;
    settings.samplesMSAA           = MSAASamples::x1;
    settings.clearColor            = Vec4(0.02, 0.02, 0.02, 1.0);
    settings.enableUI              = true;
    settings.enableRaytracing      = true;
    settings.softwareAA            = SoftwareAA::FXAA;
    settings.highDynamicPrecission = FloatPrecission::F32;

    init(settings);

    m_renderer->render(m_scene);

    Core::ITexture* texture1 = m_renderer->capture_texture(7);
    Core::ITexture* texture2 = m_renderer->capture_texture(8);
    Core::ITexture* texture3 = m_renderer->capture_texture(6);
    Core::ITexture* texture4 = m_renderer->capture_texture(10);
    Tools::Loaders::save_texture(texture1, "output_albedo.png");
    Tools::Loaders::save_texture(texture2, "output_material.png");
    Tools::Loaders::save_texture(texture3, "output_normals.hdr");
    Tools::Loaders::save_texture(texture4, "output_depth.hdr");

    m_renderer->shutdown(m_scene);
}

void Application::setup() {
    const std::string SCENE_PATH(TESTS_RESOURCES_PATH "scenes/");
    const std::string MESH_PATH(TESTS_RESOURCES_PATH "meshes/");
    const std::string TEXTURE_PATH(TESTS_RESOURCES_PATH "textures/");

    auto camera = new Camera();
    camera->set_position(Vec3(0.0f, 0.5f, -1.0f));
    camera->set_far(100.0f);
    camera->set_near(0.1f);
    camera->set_field_of_view(70.0f);

    m_scene = new Scene(camera);

    m_scene->add(new PointLight());
    m_scene->get_lights()[0]->set_position({-3.0f, 3.0f, 0.0f});
    m_scene->get_lights()[0]->set_shadow_fov(25.0f);
    m_scene->get_lights()[0]->set_shadow_target({0.0f, 0.5f, 0.0f});
    m_scene->get_lights()[0]->set_intensity(1.0f);
    m_scene->get_lights()[0]->set_shadow_bias(0.001f);
    m_scene->get_lights()[0]->set_shadow_softness(5);

    Mesh*    headMesh     = new Mesh();
    auto     skinMaterial = new PhysicalMaterial();
    TextureLDR* skinAlbedo   = new TextureLDR();
    Tools::Loaders::load_texture(skinAlbedo, TEXTURE_PATH + "perry_albedo.png", TEXTURE_FORMAT_SRGB, false);
    skinMaterial->set_albedo_texture(skinAlbedo);
    TextureLDR* skinNormal = new TextureLDR();
    Tools::Loaders::load_texture(skinNormal, TEXTURE_PATH + "perry_normal.png", TEXTURE_FORMAT_UNORM, false);
    skinMaterial->set_normal_texture(skinNormal);
    skinMaterial->set_roughness(0.5);
    headMesh->add_material(skinMaterial);
    Tools::Loaders::load_3D_file(headMesh, MESH_PATH + "lee_perry.obj", false);
    headMesh->set_name("Head");
    headMesh->set_scale(2.0f);
    headMesh->set_rotation({0.0, 180.0f, 0.0f});

    m_scene->add(headMesh);

    m_scene->set_ambient_intensity(0.1f);
    m_scene->use_IBL(false);
}
