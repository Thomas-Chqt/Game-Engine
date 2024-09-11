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

#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "Renderer/Mesh.hpp"
#include <cmath>
#include "ECS/Entity.hpp"

namespace GE
{

struct NameComponent
{
    utils::String name;

    inline operator utils::String& () { return name; }
};

struct TransformComponent
{
    math::vec3f position;
    math::vec3f rotation;
    math::vec3f scale;

    inline math::mat4x4 transform() { return math::mat4x4::translation(position) * math::mat4x4::rotation(rotation) * math::mat4x4::scale(scale); }

    inline operator math::mat4x4 () { return transform(); }
};

struct CameraComponent
{
    float fov;
    float zFar;
    float zNear;

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
};

struct MeshComponent
{
    Mesh mesh;

    inline operator Mesh& () { return mesh; }
};

struct ScriptComponent
{
    utils::UniquePtr<Entity> instance;
};

struct HierarchicalComponent
{
    Entity parent;
    Entity firstChild;
    Entity nextChild;
};

}

#endif // COMPONENTS_HPP