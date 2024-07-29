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

#include "Graphics/GraphicAPI.hpp"
#include "Math/Matrix.hpp"
#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

class Renderer
{
public:
    Renderer()                = default;
    Renderer(const Renderer&) = default;
    Renderer(Renderer&&)      = default;
    
    Renderer(utils::SharedPtr<gfx::GraphicAPI>);

    void beginScene(const math::vec3f& cameraPos, const math::mat4x4& vpMatrix);
    void endScene();

    ~Renderer() = default;

private:
    utils::SharedPtr<gfx::GraphicAPI> m_graphicAPI;

    // Scene data
    math::vec3f m_cameraPos;
    math::mat4x4 m_vpMatrix;

public:
    Renderer& operator = (const Renderer&) = default;
    Renderer& operator = (Renderer&&)      = default;
};

}

#endif // RENDERER_HPP