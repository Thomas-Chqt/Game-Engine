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
    EditorCamera(const EditorCamera&) = delete;
    EditorCamera(EditorCamera&&)      = delete;

    void setupFreeCam();

    Renderer::Camera getRendererCam();

    ~EditorCamera();

private:
    math::vec3f m_position;
    math::vec3f m_rotation;

public:
    EditorCamera& operator = (const EditorCamera&) = delete;
    EditorCamera& operator = (EditorCamera&&)      = delete;
};

}

#endif // EDITORCAMERA_HPP