/*
 * ---------------------------------------------------
 * DefaultRenderMethod.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/30 12:39:37
 * ---------------------------------------------------
 */

#ifndef DEFAULTRENDERMETHOD_HPP
#define DEFAULTRENDERMETHOD_HPP

#include "Graphics/GraphicPipeline.hpp"
#include "Graphics/Shader.hpp"
#include "Renderer/RenderMethod.hpp"
#include "UtilsCPP/Types.hpp"

namespace GE
{

class DefaultRenderMethod : public RenderMethod
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
    DefaultRenderMethod()                           = default;
    DefaultRenderMethod(const DefaultRenderMethod&) = delete;
    DefaultRenderMethod(DefaultRenderMethod&&)      = delete;

    void setGraphicAPI(const utils::SharedPtr<gfx::GraphicAPI>&) override;

    gfx::Shader::Descriptor vertexShaderDescriptor() override;
    gfx::Shader::Descriptor fragmentShaderDescriptor() override;
    gfx::GraphicPipeline::Descriptor graphicPipelineDescriptor() override;

    inline utils::uint64 vpMatrixBufferIdx() override { return m_vpMatrixBufferIdx; }
    inline utils::uint64 modelMatrixBufferIdx() override { return m_modelMatrixBufferIdx; }
    inline utils::uint64 lightsBufferIdx() override { return m_lightsBufferIdx; }

    ~DefaultRenderMethod() override = default;

private:
    utils::uint64 m_vpMatrixBufferIdx;
    utils::uint64 m_modelMatrixBufferIdx;
    utils::uint64 m_lightsBufferIdx;

public:
    DefaultRenderMethod& operator = (const DefaultRenderMethod&) = delete;
    DefaultRenderMethod& operator = (DefaultRenderMethod&&)      = delete;
};

}

#endif // DEFAULTRENDERMETHOD_HPP