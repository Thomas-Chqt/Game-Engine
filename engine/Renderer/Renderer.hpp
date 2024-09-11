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
#include "Graphics/RenderTarget.hpp"
#include "Math/Matrix.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "Graphics/BufferInstance.hpp"
#include "ECS/Components.hpp"
#include "ECS/ECSView.hpp"

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
    Renderer()                = delete;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;

    Renderer(const utils::SharedPtr<gfx::GraphicAPI>&);
    
    inline gfx::GraphicAPI& graphicAPI() { return *m_graphicAPI; }
    
    inline void setOnImGuiRender(const utils::Func<void(void)>& f) { m_onImGuiRender = f; }

    void beginScene(const Renderer::Camera&, const utils::SharedPtr<gfx::RenderTarget>&);

    void addRenderable(const Renderer::Renderable&);
    void addPointLight(const Renderer::PointLight&);

    void addAllRenderables(ECSView<TransformComponent, MeshComponent>);
    void addAllLights(ECSView<TransformComponent, LightComponent>);

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
    utils::SharedPtr<gfx::GraphicAPI> m_graphicAPI;
    utils::Func<void(void)> m_onImGuiRender;
    utils::SharedPtr<gfx::GraphicPipeline> m_gfxPipeline; // temporary

    utils::SharedPtr<gfx::RenderTarget> m_renderTarget;
    gfx::BufferInstance<math::mat4x4> m_vpMatrix;
    gfx::BufferInstance<LightsBuffer> m_lightsBuffer;
    utils::Array<Renderable> m_renderables;

public:
    Renderer& operator = (const Renderer&) = delete;
    Renderer& operator = (Renderer&&)      = delete;
};

}

#endif // RENDERER_HPP