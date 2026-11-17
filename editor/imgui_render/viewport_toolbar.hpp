#pragma once

#include <cstdint>
#include <imgui.h>

namespace GE_Editor
{
enum class GizmoTool : uint8_t { move, rotate, scale };
enum class GizmoSpace : uint8_t { local, world };

struct GizmoSettings
{
    GizmoTool tool = GizmoTool::move;
    GizmoSpace space = GizmoSpace::local;
};

ImVec2 viewportToolbarSize();
bool renderViewportToolbar(GizmoSettings& settings, ImVec2 topRight, bool dragging);
} // namespace GE_Editor
