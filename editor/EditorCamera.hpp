/*
 * ---------------------------------------------------
 * EditorCamera.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 *
 * Camera used to view the scene in the editor,
 * it can forward to the scene's active camera in
 * case of this scene or game running in the editor
 *
 */

#ifndef EDITORCAMERA_HPP
#define EDITORCAMERA_HPP

#include <Game-Engine/Scene.hpp>
#include <Game-Engine/ICamera.hpp>

#include <glm/glm.hpp>

namespace GE_Editor
{

class EditorCamera
{
public:
    EditorCamera() = default;
    EditorCamera(const EditorCamera&) = delete;
    EditorCamera(EditorCamera&&) = delete;

    inline glm::vec3 position() const { return m_position; }
    glm::mat4 viewProjectionMatrix(float aspectRatio) const;

private:
    glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_rotation = { 0.0f, 0.0f, 0.0f };
    float m_fov = glm::radians(60.0f);
    float m_zFar = 1000.0f;
    float m_zNear = 0.1f;

public:
    EditorCamera& operator=(const EditorCamera&) = delete;
    EditorCamera& operator=(EditorCamera&&) = delete;
};

} // namespace GE_Editor

#endif
