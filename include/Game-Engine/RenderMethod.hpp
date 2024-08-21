/*
 * ---------------------------------------------------
 * RenderMethod.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/20 17:42:48
 * ---------------------------------------------------
 */

#ifndef RENDERMETHOD_HPP
#define RENDERMETHOD_HPP

#include "Graphics/Shader.hpp"
#include "Graphics/GraphicPipeline.hpp"

namespace GE
{

class RenderMethod
{
public:
    RenderMethod(const RenderMethod&) = default;
    RenderMethod(RenderMethod&&)      = default;

    void build();
    void use();

    virtual gfx::Shader::Descriptor vertexShaderDescriptor() = 0;
    virtual gfx::Shader::Descriptor fragmentShaderDescriptor() = 0;
    virtual gfx::GraphicPipeline::Descriptor graphicPipelineDescriptor() = 0;

    virtual ~RenderMethod() = default;

protected:
    RenderMethod() = default;
    
public:
    RenderMethod& operator = (const RenderMethod&) = default;
    RenderMethod& operator = (RenderMethod&&)      = default;
};

}

#endif // RENDERMETHOD_HPP