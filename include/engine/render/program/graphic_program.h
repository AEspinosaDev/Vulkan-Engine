/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef GFX_SHADER_PROGRAM
#define GFX_SHADER_PROGRAM

#include <engine/render/program/shader_program.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

class GraphicShaderProgram final : public ShaderProgram
{

    Graphics::GraphicShaderPass m_shaderpass = {};

public:
    GraphicShaderProgram( std::string name, std::string glslPath, const std::vector<UniformBinding>& uniformBindings, const Graphics::GraphicPipelineConfig& config = {} )
        : ShaderProgram( name, glslPath, uniformBindings ) {
        m_shaderpass.config = config;
    }

    void        compile( const std::shared_ptr<Graphics::Device>& device ) override;
    void        bind( Frame& frame ) override;
    void        bind_uniform_set( uint32_t set, Frame& frame ) override;
    void        bind_uniform_set( uint32_t set, Frame& frame, const std::vector<uint32_t>& offsets ) override; // if needed
    inline bool is_graphics() const override { return true; }
    bool        is_compute() const override {
        { return false; }
    }
    void cleanup() override;

    const Graphics::GraphicPipelineConfig& get_config() const { return m_shaderpass.config; }
};

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END
#endif