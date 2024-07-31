/*
 * ---------------------------------------------------
 * RenderMethod.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/30 12:25:45
 * ---------------------------------------------------
 */

#ifndef RENDERMETHOD_HPP
#define RENDERMETHOD_HPP

#include "Graphics/GraphicAPI.hpp"
#include "Graphics/GraphicPipeline.hpp"
#include "Graphics/Shader.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/Types.hpp"

namespace GE
{

class RenderMethod
{
public:
    struct Vertex
    {
        math::vec3f pos;
        math::vec2f uv;
        math::vec3f normal;
        math::vec3f tangent;
    };

public:
    RenderMethod(const RenderMethod&) = delete;
    RenderMethod(RenderMethod&&)      = delete;

    void build(gfx::GraphicAPI&);

    void use();

    void setVPMatrixBuffer(const utils::SharedPtr<gfx::Buffer>&);
    void setLightsBuffer(const utils::SharedPtr<gfx::Buffer>&);
    void setModelMatrixBuffer(const utils::SharedPtr<gfx::Buffer>&);

    virtual gfx::Shader::Descriptor vertexShaderDescriptor() = 0;
    virtual gfx::Shader::Descriptor fragmentShaderDescriptor() = 0;
    virtual gfx::GraphicPipeline::Descriptor graphicPipelineDescriptor() = 0;

    virtual utils::uint64 vpMatrixBufferIdx() = 0;
    virtual utils::uint64 modelMatrixBufferIdx() = 0;
    virtual utils::uint64 lightsBufferIdx() = 0;

    virtual ~RenderMethod() = default;

protected:
    RenderMethod() = default;

    utils::SharedPtr<gfx::GraphicPipeline> m_graphicPipeline;

private:
    gfx::GraphicAPI* m_graphicAPI = nullptr;

public:
    RenderMethod& operator = (const RenderMethod&) = delete;
    RenderMethod& operator = (RenderMethod&&)      = delete;
};

}

#endif // RENDERMETHOD_HPP