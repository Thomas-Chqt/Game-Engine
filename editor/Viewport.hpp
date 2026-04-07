/*
 * ---------------------------------------------------
 * Viewport.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 *
 */

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

#include "EditorCamera.hpp"

#include <Game-Engine/Entity.hpp>
#include <Game-Engine/ICamera.hpp>

#include <glm/glm.hpp>

#include <utility>
#include <variant>

namespace GE_Editor
{

class Viewport
{
public:
    const std::pair<uint32_t, uint32_t>& size() const { return m_size; }
    const GE::ICamera* camera() const { return &m_camera; }

    void setSize(const std::pair<uint32_t, uint32_t>&);

    void setCamera(const EditorCamera* editorCamera) { m_camera.m_cameraBackend = editorCamera; }
    void setCamera(const GE::const_Entity& entity) { m_camera.m_cameraBackend = entity; }

private:
    struct Camera : public GE::ICamera
    {
    public:
        glm::vec3 position() const override;
        glm::mat4 viewProjectionMatrix() const override;

    private:
        friend Viewport;
        using CameraBackend = std::variant<const EditorCamera*, GE::const_Entity>;
        float m_aspectRatio = 16.0f/9.0f;
        CameraBackend m_cameraBackend;
    };

    std::pair<uint32_t, uint32_t> m_size = {0, 0};
    Camera m_camera;
};

}

#endif
