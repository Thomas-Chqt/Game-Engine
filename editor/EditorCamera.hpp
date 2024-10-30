/*
 * ---------------------------------------------------
 * EditorCamera.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 13:51:34
 * ---------------------------------------------------
 */

#ifndef EDITORCAMERA_HPP
#define EDITORCAMERA_HPP

#include "Math/Vector.hpp"
#include "Renderer/Renderer.hpp"

namespace GE
{

class EditorCamera
{
public:
    EditorCamera()                    = default;
    EditorCamera(const EditorCamera&) = default;
    EditorCamera(EditorCamera&&)      = default;

    void rotate(math::vec2f);
    void move(math::vec2f);

    Renderer::Camera getRendererCam();
    
    ~EditorCamera();

private:
    math::vec3f m_position;
    math::vec3f m_rotation;

public:
    EditorCamera& operator = (const EditorCamera&) = default;
    EditorCamera& operator = (EditorCamera&&)      = default;
};

}

#endif // EDITORCAMERA_HPP