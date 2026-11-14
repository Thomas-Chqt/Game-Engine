#include "viewport_toolbar.hpp"
#include "EditorTheme.hpp"

#include <cmath>
#include <array>

namespace GE_Editor
{
namespace
{
// Icons use the same 24-unit design grid and stroke at their actual display size.
void toolIcon(GizmoTool tool, ImVec2 center, ImU32 color)
{
    auto* draw = ImGui::GetWindowDrawList();
    const float size = Theme::viewportToolIconSize;
    const float stroke = Theme::smallIconStroke;
    const ImVec2 origin(std::floor(center.x-size*.5f), std::floor(center.y-size*.5f));
    const auto p = [&](float x, float y) { return ImVec2(origin.x+x*size/24, origin.y+y*size/24); };
    const auto line = [&](float x, float y, float a, float b) { draw->AddLine(p(x,y), p(a,b), color, stroke); };
    if (tool == GizmoTool::move) {
        line(5,12,19,12); line(12,5,12,19);
        draw->AddTriangleFilled(p(2,12), p(6,9), p(6,15), color);
        draw->AddTriangleFilled(p(22,12), p(18,9), p(18,15), color);
        draw->AddTriangleFilled(p(12,2), p(9,6), p(15,6), color);
        draw->AddTriangleFilled(p(12,22), p(9,18), p(15,18), color);
    }
    else if (tool == GizmoTool::rotate) {
        draw->PathArcTo(p(12,12), size*8.5f/24, .15f*3.14159265f, 1.85f*3.14159265f, 32);
        draw->PathStroke(color, 0, stroke);
        const std::array<ImVec2, 3> head = {p(15,9), p(20,9), p(20,4)};
        draw->AddPolyline(head.data(), head.size(), color, 0, stroke);
    }
    else {
        draw->AddRect(p(3,15), p(9,21), color, 0, 0, stroke);
        line(9,15,20,4);
        const std::array<ImVec2, 3> head = {p(13,4), p(20,4), p(20,11)};
        draw->AddPolyline(head.data(), head.size(), color, 0, stroke);
    }
}

void spaceIcon(bool world, ImVec2 center, ImU32 color)
{
    auto* draw = ImGui::GetWindowDrawList();
    const float size = Theme::viewportToolIconSize;
    const float stroke = Theme::smallIconStroke;
    const ImVec2 origin(std::floor(center.x-size*.5f), std::floor(center.y-size*.5f));
    const auto p = [&](float x, float y) { return ImVec2(origin.x+x*size/24, origin.y+y*size/24); };
    const auto line = [&](float x, float y, float a, float b) { draw->AddLine(p(x,y), p(a,b), color, stroke); };
    if (world) {
        draw->AddCircle(p(12,12), size*9/24, color, 32, stroke);
        for (int i=0; i<32; ++i) {
            const float a = i*6.2831853f/32;
            draw->PathLineTo(p(12+std::cos(a)*4.5f, 12+std::sin(a)*9));
        }
        draw->PathStroke(color, ImDrawFlags_Closed, stroke);
        line(3,12,21,12);
    }
    else {
        line(7,17,7,5); line(7,17,19,17); line(7,17,16,8);
        draw->AddTriangleFilled(p(7,2), p(4.5f,6), p(9.5f,6), color);
        draw->AddTriangleFilled(p(22,17), p(18,14.5f), p(18,19.5f), color);
        draw->AddTriangleFilled(p(18,6), p(13.5f,7), p(17,10.5f), color);
    }
}

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

void updateGizmoShortcuts(GizmoSettings& settings, bool enabled)
{
    const auto& io = ImGui::GetIO();
    if (!enabled || io.WantTextInput || ImGui::IsAnyItemActive()
        || io.KeyCtrl || io.KeySuper || io.KeyAlt || io.KeyShift
        || ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) return;

    if (ImGui::Shortcut(ImGuiKey_W, ImGuiInputFlags_RouteGlobal)) settings.tool = GizmoTool::move;
    if (ImGui::Shortcut(ImGuiKey_E, ImGuiInputFlags_RouteGlobal)) settings.tool = GizmoTool::rotate;
    if (ImGui::Shortcut(ImGuiKey_R, ImGuiInputFlags_RouteGlobal)) settings.tool = GizmoTool::scale;
    if (ImGui::Shortcut(ImGuiKey_Space, ImGuiInputFlags_RouteGlobal))
        settings.tool = static_cast<GizmoTool>((static_cast<int>(settings.tool)+1)%3);
    if (settings.tool != GizmoTool::scale && ImGui::Shortcut(ImGuiKey_X, ImGuiInputFlags_RouteGlobal))
        settings.space = settings.space == GizmoSpace::local ? GizmoSpace::world : GizmoSpace::local;
}

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
        for (int i=0; i<3; ++i) {
            if (i) ImGui::SameLine();
            const auto tool = static_cast<GizmoTool>(i);
            ImGui::PushID(i);
            if (selectedButton("##tool", settings.tool == tool, ImVec2(button, button))) settings.tool = tool;
            const auto min = ImGui::GetItemRectMin();
            toolIcon(tool, ImVec2(min.x+button*.5f, min.y+button*.5f),
                ImGui::GetColorU32(Theme::viewportToolText));
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
        spaceIcon(world, ImVec2(min.x+button*.5f, min.y+button*.5f), ImGui::GetColorU32(Theme::viewportToolText));
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
