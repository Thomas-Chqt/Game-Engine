/*
 * ---------------------------------------------------
 * Components.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/18 17:35:21
 * ---------------------------------------------------
 */

#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ECS/Entity.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/String.hpp"
#include <cmath>

namespace GE
{

struct NameComponent
{
    utils::String name;

    NameComponent(const utils::String& n) : name(n) {};

    inline operator utils::String& () { return name; }
};

struct HierarchyComponent
{
    Entity parent;
    Entity firstChild;
    Entity nextChild;

    HierarchyComponent() = default;
};

struct TransformComponent
{
    math::vec3f position;
    math::vec3f rotation;
    math::vec3f scale;

    TransformComponent(const math::vec3f& p, const math::vec3f& r, const math::vec3f& s)
        : position(p), rotation(r), scale(s) {}

    inline math::mat4x4 transform() { return math::mat4x4::translation(position) * math::mat4x4::rotation(rotation) * math::mat4x4::scale(scale); }

    inline operator math::mat4x4 () { return transform(); }
};

struct CameraComponent
{
    float fov;
    float zFar;
    float zNear;

    CameraComponent(float f, float zf, float zn)
        : fov(f), zFar(zf), zNear(zn) {}

    math::mat4x4 projectionMatrix()
    {
        float zs = zFar / (zFar - zNear);
        float ys = 1.0F / std::tan(fov * 0.5F);
        float xs = ys; // (ys / aspectRatio)

        return math::mat4x4(xs,  0,  0,           0,
                             0, ys,  0,           0,
                             0,  0, zs, -zNear * zs,
                             0,  0,  1,           0);
    }
};

struct LightComponent
{
    enum class Type { point } type;
    math::rgb color;
    float intentsity;

    LightComponent(Type t, math::rgb c, float i)
        : type(t), color(c), intentsity(i) {}
};

}

#endif // COMPONENTS_HPP