/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMPUTE_SHADER_PROGRAM
#define COMPUTE_SHADER_PROGRAM

#include <engine/render/program/shader_program.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

class ComputeShaderProgram final : public ShaderProgram
{
    Graphics::ComputeShaderPass m_shaderpass = {};

public:
    ComputeShaderProgram( std::string name, std::string glslPath, const std::vector<UniformBinding>& uniformBindings )
        : ShaderProgram( name, glslPath, uniformBindings ) {
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
};

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END
#endif