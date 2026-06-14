/*
 * ---------------------------------------------------
 * EditorCamera.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "EditorCamera.hpp"

#include <Game-Engine/Scene.hpp>
#include <Game-Engine/Components.hpp>

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

namespace GE_Editor
{

namespace
{

constexpr float kMoveStepPerFrame = 5.0f / 60.0f;
constexpr float kRotateStepPerFrame = 1.5f / 60.0f;
constexpr float kMaxPitch = std::numbers::pi_v<float> * 0.5f - 0.001f;

bool isFinite(const glm::mat4& matrix)
{
    for (glm::length_t col = 0; col < matrix.length(); col++)
    {
        for (glm::length_t row = 0; row < matrix[col].length(); row++)
        {
            if (!std::isfinite(matrix[col][row]))
                return false;
        }
    }
    return true;
}

}

EditorCamera::EditorCamera(glm::vec3 pos, glm::vec3 rot)
    : m_position(pos)
    , m_rotation(rot)
{
}

glm::mat4 EditorCamera::viewMatrix() const
{
    auto rotationMat = rotationMatrix();

    glm::vec3 pos = m_position;
    glm::vec3 dir = rotationMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    glm::vec3 up = rotationMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    return glm::lookAt(pos, pos + dir, up);
}

glm::mat4 EditorCamera::projectionMatrix(float aspectRatio) const
{
    return glm::perspective(m_fov, aspectRatio, m_zNear, m_zFar);
}

glm::mat4 EditorCamera::viewProjectionMatrix(float aspectRatio) const
{

    return projectionMatrix(aspectRatio) * viewMatrix();
}

void EditorCamera::setViewMatrix(const glm::mat4& viewMatrix)
{
    if (!isFinite(viewMatrix) || std::abs(glm::determinant(viewMatrix)) < 0.000001f)
        return;

    const glm::mat4 cameraTransform = glm::inverse(viewMatrix);
    if (!isFinite(cameraTransform))
        return;

    const glm::mat3 rotationMatrix(cameraTransform);
    const float sinPitch = std::clamp(-rotationMatrix[2][1], -1.0f, 1.0f);

    m_position = glm::vec3(cameraTransform[3]);
    m_rotation.x = std::asin(sinPitch);
    if (std::abs(std::cos(m_rotation.x)) > 0.0001f)
    {
        m_rotation.y = std::atan2(rotationMatrix[2][0], rotationMatrix[2][2]);
        m_rotation.z = std::atan2(rotationMatrix[0][1], rotationMatrix[1][1]);
    }
    else
    {
        m_rotation.y = sinPitch > 0.0f
            ? std::atan2(rotationMatrix[1][0], rotationMatrix[0][0])
            : std::atan2(-rotationMatrix[1][0], rotationMatrix[0][0]);
        m_rotation.z = 0.0f;
    }

    m_rotation.x = std::clamp(m_rotation.x, -kMaxPitch, kMaxPitch);
}

void EditorCamera::onMoveInput(const glm::vec2& value)
{
    m_position += glm::vec3(rotationMatrix() * glm::vec4(glm::vec3{ value.x, 0.0f, -value.y } * kMoveStepPerFrame, 0.0f));
}

void EditorCamera::onRotationInput(const glm::vec2& value)
{
    m_rotation.y -= value.y * kRotateStepPerFrame;
    m_rotation.x += value.x * kRotateStepPerFrame;
}

glm::mat4 EditorCamera::rotationMatrix() const
{
    auto rotationMat = glm::mat4x4(1.0f);
    rotationMat = glm::rotate(rotationMat, m_rotation.y, glm::vec3(0, 1, 0));
    rotationMat = glm::rotate(rotationMat, m_rotation.x, glm::vec3(1, 0, 0));
    rotationMat = glm::rotate(rotationMat, m_rotation.z, glm::vec3(0, 0, 1));
    return rotationMat;
}

}
