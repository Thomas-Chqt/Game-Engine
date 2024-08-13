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
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"
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
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;
    
    static inline void init() { s_sharedInstance = utils::UniquePtr<Renderer>(new Renderer()); }
    static inline Renderer& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    void setWindow(const utils::SharedPtr<gfx::Window>&);

    void beginScene(const Renderer::Camera&);

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
    Renderer() = default;

    inline static utils::UniquePtr<Renderer> s_sharedInstance;

    utils::SharedPtr<gfx::Window> m_window;
    utils::SharedPtr<gfx::GraphicAPI> m_graphicAPI;

    // Scene data
    gfx::BufferInstance<math::mat4x4> m_vpMatrix;
    gfx::BufferInstance<LightsBuffer> m_lightsBuffer;

    utils::Array<Renderable> m_renderables;
    
public:
    Renderer& operator = (const Renderer&) = delete;
    Renderer& operator = (Renderer&&)      = delete;
};

}

#endif // RENDERER_HPP