/*
 * ---------------------------------------------------
 * EditorCamera.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef EDITORCAMERA_HPP
#define EDITORCAMERA_HPP

#include <Game-Engine/Scene.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace YAML
{
template<typename T>
struct convert;
}

namespace GE_Editor
{

class EditorCamera
{
public:
    EditorCamera() = default;
    EditorCamera(const EditorCamera&) = default;
    EditorCamera(EditorCamera&&) = default;

    EditorCamera(glm::vec3 pos, glm::quat rot);

    inline glm::vec3 position() const { return m_position; }
    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix(float aspectRatio) const;
    glm::mat4 viewProjectionMatrix(float aspectRatio) const;

    void setViewMatrix(const glm::mat4& viewMatrix);
    void onMoveInput(const glm::vec2& value);
    void onRotationInput(const glm::vec2& value);

    ~EditorCamera() = default;

private:
    glm::mat4 rotationMatrix() const;

    glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
    glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    float m_fov = glm::radians(60.0f);
    float m_zFar = 1000.0f;
    float m_zNear = 0.1f;

public:
    EditorCamera& operator=(const EditorCamera&) = default;
    EditorCamera& operator=(EditorCamera&&) = default;

    friend struct YAML::convert<EditorCamera>;
};

} // namespace GE_Editor

#endif
