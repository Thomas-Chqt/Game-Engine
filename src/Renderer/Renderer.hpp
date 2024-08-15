/*
 * ---------------------------------------------------
 * Renderer.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/09 15:49:29
 * ---------------------------------------------------
 */

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Graphics/Buffer.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/Window.hpp"
#include "Math/Matrix.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "Graphics/BufferInstance.hpp"

namespace GE
{

class Renderer
{
public:
    struct Camera
    {
        math::mat4x4 viewMatrix;
        math::mat4x4 projectionMatrix;
    };

    struct Renderable
    {
        utils::SharedPtr<gfx::Buffer> vertexBuffer;
        utils::SharedPtr<gfx::Buffer> indexBuffer;
        utils::SharedPtr<gfx::Buffer> modelMatrix;
    };

    struct PointLight
    {
        math::vec3f pos;
        math::rgb color;
        float intentsity;
    };

    struct Vertex
    {
        math::vec3f pos;
        math::vec2f uv;
        math::vec3f normal;
        math::vec3f tangent;
    };

public:
    Renderer();
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;
    
    inline void setOnImGuiRender(const utils::Func<void(void)>& f) { m_onImGuiRender = f; }

    void beginScene(const Renderer::Camera&, const gfx::Window&);

    void addRenderable(const Renderer::Renderable&);
    void addPointLight(const Renderer::PointLight&);

    void endScene();

    void render();

    ~Renderer() = default;

private:
    struct LightsBuffer
    {
        PointLight pointLights[32];
        utils::uint32 pointLightCount;
    };

private:
    utils::SharedPtr<gfx::GraphicPipeline> gfxPipeline;

    gfx::GraphicAPI& m_graphicAPI;

    gfx::BufferInstance<math::mat4x4> m_vpMatrix;
    gfx::BufferInstance<LightsBuffer> m_lightsBuffer;

    utils::Array<Renderable> m_renderables;

    utils::Func<void(void)> m_onImGuiRender;
    
public:
    Renderer& operator = (const Renderer&) = delete;
    Renderer& operator = (Renderer&&)      = delete;
};

}

#endif // RENDERER_HPP