/*
 * ---------------------------------------------------
 * Renderer.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/29 12:10:05
 * ---------------------------------------------------
 */

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Graphics/Buffer.hpp"
#include "Graphics/BufferInstance.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "Renderer/DefaultRenderMethod.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

class Renderer
{
public:
    struct Renderable
    {
        utils::SharedPtr<gfx::Buffer> vertexBuffer;
        utils::SharedPtr<gfx::Buffer> indexBuffer;
        utils::SharedPtr<gfx::Buffer> modelMatrix;
    };

public:
    Renderer()                = default;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;
    
    void setGraphicAPI(const utils::SharedPtr<gfx::GraphicAPI>& api);

    void beginScene(const math::mat4x4& vpMatrix);

    void addPointLight(const math::vec3f& pos, const math::rgb& color, float intentsity);
    inline void addRenderable(const Renderable& renderable) { m_renderables.append(renderable); }

    void endScene();

    ~Renderer() = default;

private:
    utils::SharedPtr<gfx::GraphicAPI> m_graphicAPI;
    DefaultRenderMethod m_defaultRenderMethod;

    gfx::BufferInstance<math::mat4x4> m_vpMatrixBuffer;
    gfx::BufferInstance<RenderMethod::LightsBuffer> m_lightsBuffer;

    utils::Array<Renderable> m_renderables;

public:
    Renderer& operator = (const Renderer&) = delete;
    Renderer& operator = (Renderer&&)      = delete;
};

}

#endif // RENDERER_HPP