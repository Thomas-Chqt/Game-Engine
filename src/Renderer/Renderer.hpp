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
#include "Game-Engine/RenderMethod.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"

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
    Renderer()                = delete;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;

    Renderer(gfx::GraphicAPI&);
    
    void beginScene(const math::mat4x4& vpMatrix);

    inline void addPointLight(const math::vec3f& pos, const math::rgb& color, float intentsity) { m_lightsBuffer.content().pointLights[m_lightsBuffer.content().pointLightCount++] = {pos, color, intentsity}; }
    inline void addRenderable(const Renderable& renderable) { m_renderables.append(renderable); }

    void endScene();

    ~Renderer() = default;

private:
    struct LightsBuffer
    {
        struct {
            math::vec3f pos;
            math::rgb color;
            float intentsity;
        }
        pointLights[32];
        utils::uint32 pointLightCount;
    };

    void useRenderMethod(RenderMethod*);

    gfx::GraphicAPI& m_graphicAPI;
    utils::Array<utils::UniquePtr<RenderMethod>> m_renderMethods;
    RenderMethod* m_defaultRenderMethod;

    gfx::BufferInstance<math::mat4x4> m_vpMatrixBuffer;
    gfx::BufferInstance<LightsBuffer> m_lightsBuffer;

    // Scene time
    utils::Array<Renderable> m_renderables;
    RenderMethod* m_usedRenderMethod = nullptr;

public:
    Renderer& operator = (const Renderer&) = delete;
    Renderer& operator = (Renderer&&)      = delete;
};

}

#endif // RENDERER_HPP