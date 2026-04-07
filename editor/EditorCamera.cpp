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

#include <cassert>

namespace GE_Editor
{

glm::mat4 EditorCamera::viewProjectionMatrix(float aspectRatio) const
{
    auto rotationMat = glm::mat4x4(1.0f);
    rotationMat = glm::rotate(rotationMat, m_rotation.y, glm::vec3(0, 1, 0));
    rotationMat = glm::rotate(rotationMat, m_rotation.x, glm::vec3(1, 0, 0));
    rotationMat = glm::rotate(rotationMat, m_rotation.z, glm::vec3(0, 0, 1));

    glm::vec3 pos = m_position;
    glm::vec3 dir = rotationMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    glm::vec3 up = rotationMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    return glm::perspective(m_fov, aspectRatio, m_zNear, m_zFar) * glm::lookAt(pos, pos + dir, up);
}

}
