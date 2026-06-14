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
#include <Game-Engine/ICamera.hpp>

#include <glm/glm.hpp>

namespace YAML
{
template<typename T>
struct convert;
}

namespace GE_Editor
{

class EditorCamera : public GE::ICamera
{
public:
    EditorCamera() = default;
    EditorCamera(const EditorCamera&) = default;
    EditorCamera(EditorCamera&&) = default;

    EditorCamera(glm::vec3 pos, glm::vec3 rot);

    inline glm::vec3 position() const override { return m_position; }
    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix(float aspectRatio) const;
    glm::mat4 viewProjectionMatrix(float aspectRatio) const override;

    void setViewMatrix(const glm::mat4& viewMatrix);
    void onMoveInput(const glm::vec2& value);
    void onRotationInput(const glm::vec2& value);

    ~EditorCamera() override = default;

private:
    glm::mat4 rotationMatrix() const;

    glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_rotation = { 0.0f, 0.0f, 0.0f };

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
