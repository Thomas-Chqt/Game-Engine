/*
 * ---------------------------------------------------
 * Viewport.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Viewport.hpp"
#include "EditorCamera.hpp"

#include <Game-Engine/Components.hpp>
#include <Game-Engine/Entity.hpp>

#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>

namespace GE_Editor
{

void Viewport::setSize(const std::pair<uint32_t, uint32_t>& size)
{
    m_size = size;
    m_camera.m_aspectRatio = static_cast<float>(m_size.first) / static_cast<float>(m_size.second);
}

glm::vec3 Viewport::Camera::position() const
{
    return std::visit([](auto& cameraBackend) -> glm::vec3
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(cameraBackend)>, const EditorCamera*>)
        {
            return cameraBackend->position();
        }
        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(cameraBackend)>, GE::const_Entity>)
        {
            assert(cameraBackend.template has<GE::TransformComponent>() && cameraBackend.template has<GE::CameraComponent>());
            return cameraBackend.worldTransform()[3];
        }
        else
            std::unreachable();
    },
    m_cameraBackend);
}

glm::mat4 Viewport::Camera::viewProjectionMatrix() const
{
    return std::visit([this](auto cameraBackend) -> glm::mat4
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(cameraBackend)>, const EditorCamera*>)
        {
            return cameraBackend->viewProjectionMatrix(m_aspectRatio);
        }
        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(cameraBackend)>, GE::const_Entity>)
        {
            assert(cameraBackend.template has<GE::TransformComponent>() && cameraBackend.template has<GE::CameraComponent>());
            auto& cameraTransform = cameraBackend.template get<GE::TransformComponent>();

            auto rotationMat = glm::mat4x4(1.0f);
            rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.y, glm::vec3(0, 1, 0));
            rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.x, glm::vec3(1, 0, 0));
            rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.z, glm::vec3(0, 0, 1));

            glm::vec3 pos = position();
            glm::vec3 dir = rotationMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            glm::vec3 up = rotationMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

            return cameraBackend.template get<GE::CameraComponent>().projectionMatrix(m_aspectRatio) * glm::lookAt(pos, pos + dir, up);
        }
        else
            std::unreachable();
    },
    m_cameraBackend);
}

}
