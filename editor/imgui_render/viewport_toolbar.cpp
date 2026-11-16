#include "viewport_toolbar.hpp"
#include "EditorTheme.hpp"
#include "icons.hpp"

#include <array>

namespace GE_Editor
{
namespace
{
bool selectedButton(const char* id, bool selected, ImVec2 size)
{
    ImGui::PushStyleColor(ImGuiCol_Button, selected ? Theme::viewportToolSelected : ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::viewportToolHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::viewportToolSelected);
    const bool pressed = ImGui::Button(id, size);
    ImGui::PopStyleColor(3);
    return pressed;
}
} // namespace

ImVec2 viewportToolbarSize()
{
    return ImVec2(Theme::viewportToolButtonSize*4 + Theme::viewportToolSpacing*4 + 1 + Theme::viewportToolbarPadding.x*2,
        Theme::viewportToolButtonSize + Theme::viewportToolbarPadding.y*2);
}

bool renderViewportToolbar(GizmoSettings& settings, ImVec2 topRight, bool dragging)
{
    const float button = Theme::viewportToolButtonSize;
    const float gap = Theme::viewportToolSpacing;
    ImGui::SetNextWindowPos(topRight, ImGuiCond_Always, ImVec2(1, 0));
    ImGui::SetNextWindowSize(viewportToolbarSize());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Theme::viewportToolbarPadding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, Theme::viewportToolbarRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, Theme::viewportToolButtonRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap, gap));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::viewportToolbarBackground);
    ImGui::PushStyleColor(ImGuiCol_Border, Theme::viewportToolbarBorder);
    bool hovered = false;
    if (ImGui::Begin("##Viewport tools", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus
        | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollWithMouse)) {
        hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        ImGui::BeginDisabled(dragging);
        constexpr std::array<const char*, 3> names = {"Move (W)", "Rotate (E)", "Scale (R)"};
        constexpr std::array<Icons::RenderFunction, 3> icons = {Icons::move, Icons::rotate, Icons::scale};
        for (int i=0; i<3; ++i) {
            if (i) ImGui::SameLine();
            const auto tool = static_cast<GizmoTool>(i);
            ImGui::PushID(i);
            if (selectedButton("##tool", settings.tool == tool, ImVec2(button, button))) settings.tool = tool;
            const auto min = ImGui::GetItemRectMin();
            icons[i](ImVec2(min.x+button*.5f, min.y+button*.5f), Theme::viewportToolIconSize,
                ImGui::GetColorU32(Theme::viewportToolText), Theme::smallIconStroke);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("%s\nSpace: cycle tools", names[i]);
            ImGui::PopID();
        }
        ImGui::SameLine();
        const auto separator = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddLine(separator, ImVec2(separator.x, separator.y+button),
            ImGui::GetColorU32(Theme::viewportToolbarBorder));
        ImGui::Dummy(ImVec2(1, button));
        ImGui::SameLine();
        const bool scale = settings.tool == GizmoTool::scale;
        const bool world = !scale && settings.space == GizmoSpace::world;
        ImGui::BeginDisabled(scale);
        if (selectedButton("##space", world, ImVec2(button, button)))
            settings.space = world ? GizmoSpace::local : GizmoSpace::world;
        const auto min = ImGui::GetItemRectMin();
        (world ? Icons::world : Icons::axes)(ImVec2(min.x+button*.5f, min.y+button*.5f), Theme::viewportToolIconSize,
            ImGui::GetColorU32(Theme::viewportToolText), Theme::smallIconStroke);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayShort)) {
            if (scale) ImGui::SetTooltip("Scale uses local axes.");
            else ImGui::SetTooltip("%s space (X)\nClick to use %s axes", world ? "World" : "Local", world ? "local" : "world");
        }
        ImGui::EndDisabled();
        ImGui::EndDisabled();
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(5);
    return hovered;
}
} // namespace GE_Editor
