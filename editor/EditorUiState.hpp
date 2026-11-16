#pragma once

#include "imgui_render/ui_state.hpp"
#include "imgui_render/viewport_toolbar.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <variant>

namespace GE_Editor
{
// Transient presentation state. Scene data and input/rendering coordination stay in Editor.
struct EditorUiState
{
    bool projectPropertiesOpen = false;
    bool assetManagerWindowOpen = false;
    std::filesystem::path resourceBrowserSubDir;
    GizmoSettings gizmoSettings;
    ComponentInspectorState componentInspector;
    SceneGraphState sceneGraph;

    std::string statusMessage;
    double statusUntil = 0.0;
    std::chrono::steady_clock::time_point previousFrameTime;
    // ImGui retains this address until the frame graph consumes the draw commands.
    std::variant<std::string, uint64_t> viewportTexture = std::string("viewportBackBuffer");
};
} // namespace GE_Editor
