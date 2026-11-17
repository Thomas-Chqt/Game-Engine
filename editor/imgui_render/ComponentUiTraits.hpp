#pragma once

#include "icons.hpp"

#include <Game-Engine/Components.hpp>

namespace GE_Editor
{

template<GE::Component T> struct ComponentUiTraits;

template<> struct ComponentUiTraits<GE::TransformComponent>
{
    static constexpr const char* name = "Transform";
    static constexpr const char* description = "Position, rotation, and scale in the scene.";
    static constexpr Icons::RenderFunction renderIcon = Icons::axes;
};

template<> struct ComponentUiTraits<GE::CameraComponent>
{
    static constexpr const char* name = "Camera";
    static constexpr const char* description = "View the scene through a perspective camera.";
    static constexpr Icons::RenderFunction renderIcon = Icons::camera;
};

template<> struct ComponentUiTraits<GE::LightComponent>
{
    static constexpr const char* name = "Light";
    static constexpr const char* description = "Illuminate the scene with a point or directional light.";
    static constexpr Icons::RenderFunction renderIcon = Icons::light;
};

template<> struct ComponentUiTraits<GE::ScriptComponent>
{
    static constexpr const char* name = "Script";
    static constexpr const char* description = "Attach gameplay behavior from the project's script library.";
    static constexpr Icons::RenderFunction renderIcon = Icons::script;
};

template<> struct ComponentUiTraits<GE::MeshComponent>
{
    static constexpr const char* name = "Mesh";
    static constexpr const char* description = "Display a mesh from the project's assets.";
    static constexpr Icons::RenderFunction renderIcon = Icons::mesh;
};

} // namespace GE_Editor
