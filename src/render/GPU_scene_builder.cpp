#include <engine/render/GPU_scene_builder.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

void GPUSceneBuilder::build( const ptr<Graphics::Device>& device,
                             Graphics::Frame* const       currentFrame,
                             Core::Scene* const           scene,
                             Extent2D                     displayExtent,
                             bool                         raytracingEnabled,
                             bool                         temporalFiltering ) {
    update_global_data( device, currentFrame, scene, displayExtent, temporalFiltering );
    update_object_data( device, currentFrame, scene, displayExtent, raytracingEnabled );
}

void GPUSceneBuilder::destroy( Core::Scene* const scene ) {
    clean_scene( scene );
}

void GPUSceneBuilder::update_global_data( const ptr<Graphics::Device>& device,
                                          Graphics::Frame* const       currentFrame,
                                          Core::Scene* const           scene,
                                          Extent2D                     displayExtent,
                                          bool                         jitterCamera ) {
    PROFILING_EVENT()
    /*
    CAMERA UNIFORMS LOAD
    */
    Core::Camera* camera = scene->get_active_camera();
    if ( camera->is_dirty() )
        camera->set_projection( displayExtent.width, displayExtent.height );
    Core::Camera::GPUPayload camData;
    camData.view     = camera->get_view();
    camData.proj     = camera->get_projection();
    camData.viewProj = camera->get_projection() * camera->get_view();
    /*Inversed*/
    camData.invView     = math::inverse( camData.view );
    camData.invProj     = math::inverse( camData.proj );
    camData.invViewProj = math::inverse( camData.viewProj );
    /*Windowed*/
    const glm::mat4 W {
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
    frameIndex            = ( frameIndex + 1 ) % 16;
    camData.jitter        = jitterCamera ? Utils::get_halton_jitter( frameIndex, displayExtent.width, displayExtent.height ) : Vec2 { 0.0, 0.0 };

    /*Other intersting Camera Data*/
    camData.position     = Vec4( camera->get_position(), 0.0f );
    camData.screenExtent = { displayExtent.width, displayExtent.height };
    camData.nearPlane    = camera->get_near();
    camData.farPlane     = camera->get_far();

    currentFrame->uniformBuffers[GLOBAL_LAYOUT].upload_data( &camData, sizeof( Core::Camera::GPUPayload ), 0 );

    /*
    SCENE UNIFORMS LOAD
    */

    Core::Scene::GPUPayload sceneParams;
    sceneParams.fogParams       = { camera->get_near(), camera->get_far(), scene->get_fog_intensity(), scene->is_fog_enabled() };
    sceneParams.fogColorAndSSAO = Vec4( scene->get_fog_color(), 0.0f );
    sceneParams.SSAOtype        = 0;
    sceneParams.emphasizeAO     = false;
    sceneParams.ambientColor    = Vec4( scene->get_ambient_color(), scene->get_ambient_intensity() );
    sceneParams.useIBL          = scene->use_IBL();
    if ( scene->get_skybox() ) // If skybox
    {
        sceneParams.envRotation        = scene->get_skybox()->get_rotation();
        sceneParams.envColorMultiplier = scene->get_skybox()->get_intensity();
    }
    sceneParams.time = 0.0;

    /*Limits*/
    // UPDATE AABB OF SCENE FOR VOXELIZATION PURPOSES
    scene->update_AABB();
    Core::AABB aabb      = scene->get_AABB();
    sceneParams.maxCoord = Vec4( aabb.maxCoords, 1.0f );
    sceneParams.minCoord = Vec4( aabb.minCoords, 1.0f );

    std::vector<Core::Light*> lights = scene->get_lights();
    if ( lights.size() > ENGINE_MAX_LIGHTS )
        std::sort( lights.begin(), lights.end(), [=]( Core::Light* a, Core::Light* b ) {
            return math::length( a->get_position() - camera->get_position() ) < math::length( b->get_position() - camera->get_position() );
        } );

    size_t lightIdx { 0 };
    for ( Core::Light* l : lights )
    {
        if ( l->is_active() )
        {
            /*Check if is directIonal and behaves as SUN*/
            if ( l->get_light_type() == LightType::DIRECTIONAL && scene->get_skybox() )
            {
                Core::DirectionalLight* dirL = static_cast<Core::DirectionalLight*>( l );
                if ( dirL->use_as_sun() )
                {
                    dirL->set_direction(
                        -Core::DirectionalLight::get_sun_direction( scene->get_skybox()->get_sky_settings().sunElevationDeg, -sceneParams.envRotation - 90.0f ) );
                }
            }

            sceneParams.lightUniforms[lightIdx]          = l->get_uniforms( camera->get_view() );
            Mat4 depthProjectionMatrix                   = math::perspective( math::radians( l->get_shadow_fov() ), 1.0f, l->get_shadow_near(), l->get_shadow_far() );
            Mat4 depthViewMatrix                         = math::lookAt( l->get_position(), l->get_shadow_target(), Vec3( 0, 1, 0 ) );
            sceneParams.lightUniforms[lightIdx].viewProj = depthProjectionMatrix * depthViewMatrix;
            lightIdx++;
        }
        if ( lightIdx >= ENGINE_MAX_LIGHTS )
            break;
    }
    sceneParams.numLights = static_cast<int>( lights.size() );

    currentFrame->uniformBuffers[GLOBAL_LAYOUT].upload_data(
        &sceneParams, sizeof(  Core::Scene::GPUPayload ), device->pad_uniform_buffer_size( sizeof( Core::Camera::GPUPayload ) ) );
}
void GPUSceneBuilder::update_object_data( const ptr<Graphics::Device>& device,
                                          Graphics::Frame* const       currentFrame,
                                          Core::Scene* const           scene,
                                          Extent2D                     displayExtent,
                                          bool                         enableRT ) {

    PROFILING_EVENT()

    if ( scene->get_active_camera() && scene->get_active_camera()->is_active() )
    {
        std::vector<Core::Mesh*> meshes;
        std::vector<Core::Mesh*> blendMeshes;

        for ( Core::Mesh* m : scene->get_meshes() )
        {
            if ( m->get_material() )
                m->get_material()->get_parameters().blending ? blendMeshes.push_back( m ) : meshes.push_back( m );
        }

        // Calculate distance
        if ( !blendMeshes.empty() )
        {

            std::map<float, Core::Mesh*> sorted;
            for ( unsigned int i = 0; i < blendMeshes.size(); i++ )
            {
                float distance   = glm::distance( scene->get_active_camera()->get_position(), blendMeshes[i]->get_position() );
                sorted[distance] = blendMeshes[i];
            }

            // SECOND = TRANSPARENT OBJECTS SORTED FROM NEAR TO FAR
            for ( std::map<float, Core::Mesh*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it )
            {
                meshes.push_back( it->second );
            }
            Core::set_meshes( scene, meshes );
        }

        std::vector<Graphics::BLASInstance> BLASInstances; // RT Acceleration Structures per instanced mesh
        BLASInstances.reserve( scene->get_meshes().size() );
        unsigned int mesh_idx = 0;
        for ( Core::Mesh* m : scene->get_meshes() )
        {
            if ( m ) // If mesh exists
            {
                if ( m->is_active() &&                                                                        // Check if is active
                     m->get_geometry() &&                                                                     // Check if has geometry
                     m->get_bounding_volume()->is_on_frustrum( scene->get_active_camera()->get_frustrum() ) ) // Check if is inside frustrum
                {
                    // Offset calculation
                    uint32_t objectOffset = currentFrame->uniformBuffers[OBJECT_LAYOUT].strideSize * mesh_idx;

                    Core::Object3D::GPUPayload objectData;
                    objectData.model        = m->get_model_matrix();
                    objectData.otherParams1 = { m->affected_by_fog(), m->receive_shadows(), m->cast_shadows(), mesh_idx };
                    objectData.otherParams2 = { mesh_idx, m->get_bounding_volume()->center };
                    currentFrame->uniformBuffers[OBJECT_LAYOUT].upload_data( &objectData, sizeof(  Core::Object3D::GPUPayload ), objectOffset );

                    // Object vertex buffer setup
                    Core::Geometry* g = m->get_geometry();
                    GPUResourcePool::upload_geometry_data( device, g, enableRT && m->ray_hittable() );
                    // Add BLASS to instances list
                    if ( enableRT && m->ray_hittable() && get_BLAS( g )->handle )
                        BLASInstances.push_back( { *get_BLAS( g ), m->get_model_matrix() } );

                    // Object material setup
                    Core::IMaterial* mat = m->get_material( g->get_material_ID() );
                    if ( !mat )
                    {
                        m->add_material( Core::IMaterial::debugMaterial );
                    }
                    mat = m->get_material( g->get_material_ID() );
                    if ( mat )
                    {
                        auto textures = mat->get_textures();
                        for ( auto pair : textures )
                        {
                            Core::ITexture* texture = pair.second;
                            GPUResourcePool::upload_texture_data( device, texture );
                        }
                    }

                    // ObjectUniforms materialData;
                    Core::IMaterial::GPUPayload materialData = mat->get_uniforms();
                    currentFrame->uniformBuffers[OBJECT_LAYOUT].upload_data(
                        &materialData, sizeof(  Core::IMaterial::GPUPayload ), objectOffset + device->pad_uniform_buffer_size( sizeof(  Core::IMaterial::GPUPayload ) ) );
                }
            }
            mesh_idx++;
        }
        // CREATE TOP LEVEL (STATIC) ACCELERATION STRUCTURE
        if ( enableRT )
        {
            Graphics::TLAS* accel = get_TLAS( scene );
            if ( !accel->handle )
                device->upload_TLAS( *accel, BLASInstances );
            // Update Acceleration Structure if change in objects
            if ( accel->instances < BLASInstances.size() || scene->update_AS() )
            {
                device->wait_idle();
                device->upload_TLAS( *accel, BLASInstances );
                scene->update_AS( false );
            }
        }
    }
}

void GPUSceneBuilder::build_skybox_data( const ptr<Graphics::Device>& device, Core::Skybox* const sky ) {
    if ( sky )
    {
        GPUResourcePool::upload_geometry_data( device, sky->get_box() );
        Core::TextureHDR* envMap = sky->get_enviroment_map();
        if ( envMap && envMap->loaded_on_CPU() )
        {
            if ( !envMap->loaded_on_GPU() )
            {
                Graphics::ImageConfig   config        = {};
                Graphics::SamplerConfig samplerConfig = {};
                Core::TextureSettings   textSettings  = envMap->get_settings();
                config.format                         = textSettings.format;
                samplerConfig.anysotropicFilter       = textSettings.anisotropicFilter;
                samplerConfig.filters                 = textSettings.filter;
                samplerConfig.samplerAddressMode      = textSettings.adressMode;

                void* imgCache { nullptr };
                envMap->get_image_cache( imgCache );
                device->upload_texture_image( *get_image( envMap ), config, samplerConfig, imgCache, envMap->get_bytes_per_pixel() );
            }
        }
    }
}

void GPUSceneBuilder::clean_scene( Core::Scene* const scene ) {
    if ( scene )
    {
        for ( Core::Mesh* m : scene->get_meshes() )
        {

            Core::Geometry* g = m->get_geometry();
            GPUResourcePool::destroy_geometry_data( g );

            Core::IMaterial* mat = m->get_material( g->get_material_ID() );
            if ( mat )
            {
                auto textures = mat->get_textures();
                for ( auto pair : textures )
                {
                    Core::ITexture* texture = pair.second;
                    GPUResourcePool::destroy_texture_data( texture );
                }
            }
        }
        if ( scene->get_skybox() )
        {
            GPUResourcePool::destroy_geometry_data( scene->get_skybox()->get_box() );
            GPUResourcePool::destroy_texture_data( scene->get_skybox()->get_enviroment_map() );
        }
        get_TLAS( scene )->cleanup();
    }
}
} // namespace Render

VULKAN_ENGINE_NAMESPACE_END;
