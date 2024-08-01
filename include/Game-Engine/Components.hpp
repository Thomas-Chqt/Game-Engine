/*
 * ---------------------------------------------------
 * Components.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/29 13:15:12
 * ---------------------------------------------------
 */

#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "Game-Engine/Mesh.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"
#include <cmath>

namespace GE
{

struct DebugNameComponent
{
    utils::String name;
};

struct TransformComponent
{
    math::vec3f position;
    math::vec3f rotation;
    math::vec3f scale;

    inline math::mat4x4 modelMatrix() const
    {
        return math::mat4x4::translation(position) * math::mat4x4::scale(scale) * math::mat4x4::rotation(rotation);
    }
};

struct ViewPointComponent
{
    float fov;
    float zNear;
    float zFar;

    inline math::mat4x4 projectionMatrix(float aspectRatio) const
    {
        float zs = zFar / (zFar - zNear);
        float ys = 1.0F / std::tan(fov * 0.5F);
        float xs = ys / aspectRatio;

        return math::mat4x4(xs,  0,  0,           0,
                             0, ys,  0,           0,
                             0,  0, zs, -zNear * zs,
                             0,  0,  1,           0);
    }
};

struct ActiveViewPointComponent
{
};

struct ScriptComponent
{
    utils::Func<void(const utils::Set<int>& pressedKeys)> onFrame;
};

struct LightSourceComponent
{
    enum class Type { point };
    Type type;
    math::rgb color;
    float intensity;
};

struct RenderableComponent
{
    Mesh mesh;
};

}

#endif // COMPONENTS_HPP