#pragma once

#include <imgui.h>

namespace GE_Editor
{
enum class GizmoTool { move, rotate, scale };
enum class GizmoSpace { local, world };

struct GizmoSettings
{
    GizmoTool tool = GizmoTool::move;
    GizmoSpace space = GizmoSpace::local;
};

void updateGizmoShortcuts(GizmoSettings& settings, bool enabled);
ImVec2 viewportToolbarSize();
bool renderViewportToolbar(GizmoSettings& settings, ImVec2 topRight, bool dragging);
} // namespace GE_Editor
