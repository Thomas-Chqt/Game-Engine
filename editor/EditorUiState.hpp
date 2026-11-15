#pragma once

#include "imgui_render/ui_state.hpp"
#include "imgui_render/viewport_toolbar.hpp"

#include <Game-Engine/Components.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>

namespace GE { class Scene; }

namespace GE_Editor
{
// Transient presentation state. Scene data and input/rendering coordination stay in Editor.
struct EditorUiState
{
    bool projectPropertiesOpen = false;
    bool assetManagerWindowOpen = false;
    std::filesystem::path resourceBrowserSubDir;
    GizmoSettings gizmoSettings;
    bool gizmoCancelledUntilMouseRelease = false;
    ComponentInspectorState componentInspector;
    SceneGraphState sceneGraph;

    std::string statusMessage;
    double statusUntil = 0.0;
    std::string clipboardText;
    std::chrono::steady_clock::time_point previousFrameTime;
    // ImGui retains this address until the frame graph consumes the draw commands.
    std::variant<std::string, uint64_t> viewportTexture = std::string("viewportBackBuffer");

    void beginGizmoDrag(GE::Entity entity, const GE::TransformComponent& originalTransform);
    void commitGizmoDrag();
    void cancelGizmoDrag(GE::Scene* scene);
    std::optional<GE::Entity> gizmoDragEntity() const;

private:
    struct GizmoDrag
    {
        GE::Entity entity;
        GE::TransformComponent originalTransform;
    };
    std::optional<GizmoDrag> m_gizmoDrag;
};
} // namespace GE_Editor
