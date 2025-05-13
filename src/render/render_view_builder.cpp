#include <engine/render/render_view_builder.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

RenderView RenderViewBuilder::build(const ptr<Graphics::Device>& device,
                                    Graphics::Frame* const       currentFrame,
                                    Core::Scene* const           scene,
                                    Extent2D                     displayExtent,
                                    const Settings&              settings) {

    RenderView view{};

    update_global_data(device, currentFrame, scene, displayExtent, settings.softwareAA == SoftwareAA::TAA, view);
    update_object_data(device, currentFrame, scene, displayExtent, settings.enableRaytracing, view);

    view.commandBuffer = currentFrame->commandBuffer;

    return view;
}
void RenderViewBuilder::update_global_data(const ptr<Graphics::Device>& device,
                                           Graphics::Frame* const       currentFrame,
                                           Core::Scene* const           scene,
                                           Extent2D                     displayExtent,
                                           bool                         jitterCamera,
                                           RenderView&                  view) {
    PROFILING_EVENT()
    /*
    CAMERA UNIFORMS LOAD
    */
    Core::Camera* camera = scene->get_active_camera();
    if (camera->is_dirty())
        camera->set_projection(displayExtent.width, displayExtent.height);
    CameraUniforms camData;
    camData.view     = camera->get_view();
    camData.proj     = camera->get_projection();
    camData.viewProj = camera->get_projection() * camera->get_view();
    /*Inversed*/
    camData.invView     = math::inverse(camData.view);
    camData.invProj     = math::inverse(camData.proj);
    camData.invViewProj = math::inverse(camData.viewProj);
    /*Windowed*/
    const glm::mat4 W{
        displayExtent.width / 2.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        displayExtent.height / 2.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        displayExtent.width / 2.0f,
        displayExtent.height / 2.0f,
        0.0f,
        1.0f,
    };
    camData.unormProj = W * camData.proj;
    // camData.prevViewProj  = prevViewProj;
    static int frameIndex = 0;
    frameIndex            = (frameIndex + 1) % 16;
    camData.jitter        = jitterCamera ? Utils::get_halton_jitter(frameIndex, displayExtent.width, displayExtent.height) : Vec2{0.0, 0.0};

    /*Other intersting Camera Data*/
    camData.position     = Vec4(camera->get_position(), 0.0f);
    camData.screenExtent = {displayExtent.width, displayExtent.height};
    camData.nearPlane    = camera->get_near();
    camData.farPlane     = camera->get_far();

    currentFrame->globalBuffer.upload_data(&camData, sizeof(CameraUniforms), 0);

    /*
    SCENE UNIFORMS LOAD
    */
    SceneUniforms sceneParams;
    sceneParams.fogParams       = {camera->get_near(), camera->get_far(), scene->get_fog_intensity(), scene->is_fog_enabled()};
    sceneParams.fogColorAndSSAO = Vec4(scene->get_fog_color(), 0.0f);
    sceneParams.SSAOtype        = 0;
    sceneParams.emphasizeAO     = false;
    sceneParams.ambientColor    = Vec4(scene->get_ambient_color(), scene->get_ambient_intensity());
    sceneParams.useIBL          = scene->use_IBL();
    if (scene->get_skybox()) // If skybox
    {
        sceneParams.envRotation        = scene->get_skybox()->get_rotation();
        sceneParams.envColorMultiplier = scene->get_skybox()->get_intensity();
    }
    sceneParams.time = 0.0;

    /*Limits*/
    // UPDATE AABB OF SCENE FOR VOXELIZATION PURPOSES
    scene->update_AABB();
    Core::AABB aabb      = scene->get_AABB();
    sceneParams.maxCoord = Vec4(aabb.maxCoords, 1.0f);
    sceneParams.minCoord = Vec4(aabb.minCoords, 1.0f);

    std::vector<Core::Light*> lights = scene->get_lights();
    if (lights.size() > ENGINE_MAX_LIGHTS)
        std::sort(lights.begin(), lights.end(), [=](Core::Light* a, Core::Light* b) {
            return math::length(a->get_position() - camera->get_position()) < math::length(b->get_position() - camera->get_position());
        });

    size_t lightIdx{0};
    for (Core::Light* l : lights)
    {
        if (l->is_active())
        {
            /*Check if is directIonal and behaves as SUN*/
            if (l->get_light_type() == LightType::DIRECTIONAL && scene->get_skybox())
            {
                Core::DirectionalLight* dirL = static_cast<Core::DirectionalLight*>(l);
                if (dirL->use_as_sun())
                {
                    dirL->set_direction(
                        -Core::DirectionalLight::get_sun_direction(scene->get_skybox()->get_sky_settings().sunElevationDeg, -sceneParams.envRotation - 90.0f));
                }
            }

            sceneParams.lightUniforms[lightIdx] = l->get_uniforms(camera->get_view());
            Mat4 depthProjectionMatrix          = math::perspective(math::radians(l->get_shadow_fov()), 1.0f, l->get_shadow_near(), l->get_shadow_far());
            Mat4 depthViewMatrix                = math::lookAt(l->get_position(), l->get_shadow_target(), Vec3(0, 1, 0));
            sceneParams.lightUniforms[lightIdx].viewProj = depthProjectionMatrix * depthViewMatrix;
            lightIdx++;

            if (l->get_shadow_type() != ShadowType::RAYTRACED_SHADOW)
                view.numLights++;
        }
        if (lightIdx >= ENGINE_MAX_LIGHTS)
            break;
    }
    sceneParams.numLights = static_cast<int>(lights.size());

    currentFrame->globalBuffer.upload_data(&sceneParams, sizeof(SceneUniforms), device->pad_uniform_buffer_size(sizeof(CameraUniforms)));

    view.globalBuffer = currentFrame->globalBuffer;
}
void RenderViewBuilder::update_object_data(const ptr<Graphics::Device>& device,
                                           Graphics::Frame* const       currentFrame,
                                           Core::Scene* const           scene,
                                           Extent2D                     displayExtent,
                                           bool                         enableRT,
                                           RenderView&                  view) {

    PROFILING_EVENT()

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {
        std::vector<Core::Mesh*> meshes;
        std::vector<Core::Mesh*> blendMeshes;

        for (Core::Mesh* m : scene->get_meshes())
        {
            if (m->get_material())
                m->get_material()->get_parameters().blending ? blendMeshes.push_back(m) : meshes.push_back(m);
        }

        // Calculate distance
        if (!blendMeshes.empty())
        {

            std::map<float, Core::Mesh*> sorted;
            for (unsigned int i = 0; i < blendMeshes.size(); i++)
            {
                float distance   = glm::distance(scene->get_active_camera()->get_position(), blendMeshes[i]->get_position());
                sorted[distance] = blendMeshes[i];
            }

            // SECOND = TRANSPARENT OBJECTS SORTED FROM NEAR TO FAR
            for (std::map<float, Core::Mesh*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
            {
                meshes.push_back(it->second);
            }
            Core::set_meshes(scene, meshes);
        }

        ///////////////////////////////////////////
        ///////////////////////////////////////////
        ///////////////////////////////////////////

        view.drawCalls.reserve(scene->get_meshes().size());

        std::vector<Graphics::BLASInstance> BLASInstances; // RT Acceleration Structures per instanced mesh
        BLASInstances.reserve(scene->get_meshes().size());

        unsigned int mesh_idx = 0;
        for (Core::Mesh* m : scene->get_meshes())
        {
            if (m) // If mesh exists
            {
                if (m->is_active() && m->get_geometry())
                {
                    // Prepare draw call information

                    DrawCall       drawCall{};
                    DrawParameters drawCallParams{};
                    drawCall.culled = !m->get_bounding_volume()->is_on_frustrum(scene->get_active_camera()->get_frustrum());

                    // Offset calculation
                    uint32_t objectOffset = currentFrame->objectBuffer.strideSize * mesh_idx;

                    ObjectUniforms objectData;
                    objectData.model        = m->get_model_matrix();
                    objectData.otherParams1 = {m->affected_by_fog(), m->receive_shadows(), m->cast_shadows(), mesh_idx};
                    objectData.otherParams2 = {mesh_idx, m->get_bounding_volume()->center};
                    currentFrame->objectBuffer.upload_data(&objectData, sizeof(ObjectUniforms), objectOffset);

                    // Object vertex buffer setup
                    Core::Geometry* g = m->get_geometry();

                    drawCallParams.topology = g->get_properties().topology;

                    Resources::upload_geometry_data(device, g, enableRT && m->ray_hittable());
                    // Add BLASS to instances list
                    if (enableRT && m->ray_hittable() && get_BLAS(g)->handle)
                        BLASInstances.push_back({*get_BLAS(g), m->get_model_matrix()});

                    // Object material setup
                    Core::IMaterial* mat = m->get_material();
                    if (!mat)
                    {
                        m->add_material(Core::IMaterial::debugMaterial);
                    }
                    mat = m->get_material();
                    if (mat)
                    {
                        drawCall.shaderID = mat->get_shaderpass_ID();

                        drawCallParams.culling     = mat->get_parameters().faceCulling ? mat->get_parameters().culling : CullingMode::NO_CULLING;
                        drawCallParams.depthWrites = mat->get_parameters().depthWrite;
                        drawCallParams.depthTest   = mat->get_parameters().depthTest;

                        auto textures = mat->get_textures();
                        drawCall.textureBatch.reserve(textures.size());
                        for (auto pair : textures)
                        {
                            Core::ITexture* texture = pair.second;
                            if (texture)
                            {
                                Resources::upload_texture_data(device, texture);
                                drawCall.textureBatch.push_back({get_image(texture), (uint32_t)pair.first});
                            }
                        }
                    }
                    drawCall.indexOffset  = 0;
                    drawCall.vertexArrays = *get_VAO(g);
                    drawCall.bufferOffset = currentFrame->objectBuffer.strideSize * mesh_idx;

                    // ObjectUniforms materialData;
                    MaterialUniforms materialData = mat->get_uniforms();
                    currentFrame->objectBuffer.upload_data(
                        &materialData, sizeof(MaterialUniforms), objectOffset + device->pad_uniform_buffer_size(sizeof(MaterialUniforms)));

                    // Store draw call
                    view.drawCalls.push_back(drawCall);

                    const size_t DRAWCALL_INDEX = view.drawCalls.size() - 1;

                    mat->get_parameters().blending ? view.transparentDrawCalls.push_back(DRAWCALL_INDEX) : view.opaqueDrawCalls.push_back(DRAWCALL_INDEX);
                    if (m->cast_shadows())
                    {
                        view.shadowDrawCalls.push_back(DRAWCALL_INDEX);
                    }
                }
            }
            mesh_idx++;
        }
        // CREATE TOP LEVEL (STATIC) ACCELERATION STRUCTURE
        if (enableRT)
        {
            Graphics::TLAS* accel = get_TLAS(scene);
            if (!accel->handle)
                device->upload_TLAS(*accel, BLASInstances);
            // Update Acceleration Structure if change in objects
            if (accel->instances < BLASInstances.size() || scene->update_AS())
            {
                device->wait_idle();
                device->upload_TLAS(*accel, BLASInstances);
                scene->update_AS(false);
            }
            view.TLAS = accel;
        }
    }

    view.globalBuffer = currentFrame->objectBuffer;

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    if (scene->get_skybox())
    {
        if (scene->get_skybox()->is_active())
        {
            DrawCall skyDrawCall{};
            skyDrawCall.vertexArrays = *get_VAO(scene->get_skybox()->get_box());
            skyDrawCall.textureBatch.push_back({get_image(scene->get_skybox()->get_enviroment_map()), 0});
            
            if (scene->get_skybox()->update_enviroment())
            {
                
                Render::Resources::upload_skybox_data(device, scene->get_skybox());
                view.updateEnviroment = true;
                scene->get_skybox()->update_enviroment(false);
            }
            view.drawCalls.push_back(skyDrawCall);
            const size_t DRAWCALL_INDEX = view.drawCalls.size() - 1;
            view.enviromentDrawCall     = DRAWCALL_INDEX;
        }
    }

    view.frameIndex = currentFrame->index;
}

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END