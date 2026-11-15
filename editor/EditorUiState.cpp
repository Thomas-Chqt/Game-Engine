#include "EditorUiState.hpp"

#include <Game-Engine/Scene.hpp>

#include <ImGuizmo.h>
#include <imgui.h>

namespace GE_Editor
{
void EditorUiState::beginGizmoDrag(GE::Entity entity, const GE::TransformComponent& originalTransform)
{
    // Manipulate runs every frame; preserve the snapshot from the start of this gesture.
    if (!m_gizmoDrag)
        m_gizmoDrag.emplace(GizmoDrag{entity, originalTransform});
}

void EditorUiState::commitGizmoDrag()
{
    m_gizmoDrag.reset();
}

void EditorUiState::cancelGizmoDrag(GE::Scene* scene)
{
    if (!m_gizmoDrag) return;
    auto snapshot = *m_gizmoDrag;
    m_gizmoDrag.reset();
    if (scene && snapshot.entity.registry == &scene->ecsRegistry()
        && scene->ecsRegistry().isValidEntityID(snapshot.entity.entityId)
        && snapshot.entity.has<GE::TransformComponent>()) {
        snapshot.entity.get<GE::TransformComponent>() = snapshot.originalTransform;
        snapshot.entity.updateTransformHierarchy();
    }
    // Enable(false) clears ImGuizmo's active drag. A held mouse must not resume it.
    ImGuizmo::Enable(false);
    gizmoCancelledUntilMouseRelease = ImGui::IsMouseDown(ImGuiMouseButton_Left);
}

std::optional<GE::Entity> EditorUiState::gizmoDragEntity() const
{
    return m_gizmoDrag.transform([](const GizmoDrag& drag) { return drag.entity; });
}
} // namespace GE_Editor
