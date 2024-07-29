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

#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Set.hpp"
#include <cmath>

namespace GE
{

struct TransformComponent
{
    math::vec3f position;
    math::vec3f rotation;
    math::vec3f scale;
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

}

#endif // COMPONENTS_HPP