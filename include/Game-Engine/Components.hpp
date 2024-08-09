/*
 * ---------------------------------------------------
 * Components.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 16:46:08
 * ---------------------------------------------------
 */

#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "Game-Engine/Entity.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

struct TransformComponent
{
    math::vec3f position;
    math::vec3f rotation;
    math::vec3f scale;

    inline math::mat4x4 transform() { return math::mat4x4::translation(position) * math::mat4x4::rotation(rotation) * math::mat4x4::scale(scale); }
};

struct ScriptComponent
{
    utils::UniquePtr<ScriptableEntity> instance;
};

}

#endif // COMPONENTS_HPP