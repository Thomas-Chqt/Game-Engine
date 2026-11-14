#pragma once

#include <imgui.h>

#include <array>
#include <filesystem>

namespace GE_Editor::Theme
{
// The editor's visual settings live here. Edit, rebuild, and relaunch to preview.
// Colors use RGB bytes so the palette can be copied directly from a design tool.
inline constexpr ImVec4 rgb(int r, int g, int b, float alpha = 1.0f)
{
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, alpha);
}

inline constexpr ImVec4 withAlpha(ImVec4 color, float alpha)
{
    color.w = alpha;
    return color;
}

// Graphite surfaces, warm text, and a muted jade accent.
inline constexpr auto background = rgb(25, 28, 32);
inline constexpr auto panel = rgb(31, 35, 40);
inline constexpr auto raised = rgb(39, 44, 50);
inline constexpr auto input = rgb(23, 27, 31);
inline constexpr auto hover = rgb(49, 58, 63);
inline constexpr auto selection = rgb(43, 66, 65);
inline constexpr auto border = rgb(51, 58, 64);
inline constexpr auto text = rgb(222, 225, 221);
inline constexpr auto muted = rgb(137, 148, 151);
inline constexpr auto accent = rgb(112, 191, 171);
inline constexpr auto accentHover = rgb(146, 213, 195);
inline constexpr auto folder = rgb(206, 172, 111);
inline constexpr auto file = rgb(146, 174, 183);
inline constexpr auto orientationBackground = rgb(16, 16, 16, 96.0f / 255.0f);
inline constexpr std::array<ImVec4, 3> axes = {rgb(218, 119, 116), rgb(131, 183, 136), rgb(119, 163, 212)};

inline constexpr char fontPath[] = RESOURCE_DIR "/fonts/OpenSans-Regular.ttf";
inline constexpr float fontSize = 14.0f;
inline constexpr ImVec2 windowPadding = {10, 8};
inline constexpr ImVec2 framePadding = {7, 4};
inline constexpr ImVec2 itemSpacing = {8, 6};
inline constexpr ImVec2 itemInnerSpacing = {5, 4};
inline constexpr float rounding = 4.0f;
inline constexpr float popupRounding = 6.0f;
inline constexpr float borderSize = 1.0f;
inline constexpr float scrollbarSize = 11.0f;
inline constexpr float indentSpacing = 16.0f;
inline constexpr float dockingSeparatorSize = 3.0f;
inline constexpr float axisLineWidth = 2.0f;
inline constexpr float resourceTileSize = 64.0f;
inline constexpr float resourceTileRounding = 6.0f;
inline constexpr float resourceIconSize = 32.0f;
inline constexpr float iconStroke = 1.6f;
inline constexpr int resourceLabelLines = 2;
inline constexpr float smallIconStroke = 1.35f;
inline constexpr auto componentIconColor = rgb(166, 184, 190);
inline constexpr float componentIconSize = 16.0f;
inline constexpr float componentMenuDotSpacing = 4.0f;
inline constexpr float componentMenuDotRadius = 1.2f;
inline constexpr float componentPickerWidth = 270.0f;
inline constexpr ImVec2 componentSpacing = {8, 4};
inline constexpr ImVec2 componentHeaderPadding = {7, 2};
inline constexpr float componentIconGap = 6.0f;
inline constexpr float componentLabelWidth = 52.0f;
inline constexpr float componentAxisMinWidth = 48.0f;
inline constexpr float componentPickerRowPadding = 3.0f;
inline constexpr float hierarchyRenamePaddingX = 2.0f;
inline constexpr float hierarchyIndentSpacing = 8.0f;
inline constexpr float viewportToolButtonSize = 22.0f;
inline constexpr float viewportToolIconSize = 16.0f;
inline constexpr float viewportToolSpacing = 1.0f;
inline constexpr ImVec2 viewportToolbarPadding = {2, 2};
inline constexpr float viewportToolbarInset = 6.0f;
inline constexpr float viewportToolbarRounding = 3.0f;
inline constexpr float viewportToolButtonRounding = 2.0f;
inline constexpr auto viewportToolbarBackground = rgb(68, 71, 76, 0.98f);
inline constexpr auto viewportToolbarBorder = rgb(91, 95, 101);
inline constexpr auto viewportToolHover = rgb(91, 96, 103);
inline constexpr auto viewportToolSelected = rgb(77, 115, 141);
inline constexpr auto viewportToolText = rgb(239, 241, 243);

inline void apply()
{
    auto& style = ImGui::GetStyle();
    style = ImGuiStyle{};
    style.WindowPadding = windowPadding;
    style.FramePadding = framePadding;
    style.ItemSpacing = itemSpacing;
    style.ItemInnerSpacing = itemInnerSpacing;
    style.WindowRounding = rounding;
    style.ChildRounding = rounding;
    style.FrameRounding = rounding;
    style.PopupRounding = popupRounding;
    style.GrabRounding = rounding;
    style.TabRounding = rounding;
    style.ScrollbarRounding = rounding;
    style.WindowBorderSize = borderSize;
    style.ChildBorderSize = 0;
    style.PopupBorderSize = borderSize;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 0;
    style.TabBarBorderSize = borderSize;
    style.TabBarOverlineSize = 2;
    style.ScrollbarSize = scrollbarSize;
    style.IndentSpacing = indentSpacing;
    style.DockingSeparatorSize = dockingSeparatorSize;
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);

    // Assign every slot, including rarely used widgets, to avoid stock blue accents.
    for (auto& color : style.Colors) color = raised;
    auto& c = style.Colors;
    c[ImGuiCol_Text] = text;
    c[ImGuiCol_TextDisabled] = muted;
    c[ImGuiCol_WindowBg] = panel;
    c[ImGuiCol_ChildBg] = panel;
    c[ImGuiCol_PopupBg] = raised;
    c[ImGuiCol_Border] = border;
    c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg] = input;
    c[ImGuiCol_FrameBgHovered] = hover;
    c[ImGuiCol_FrameBgActive] = selection;
    c[ImGuiCol_TitleBg] = background;
    c[ImGuiCol_TitleBgActive] = raised;
    c[ImGuiCol_TitleBgCollapsed] = background;
    c[ImGuiCol_MenuBarBg] = background;
    c[ImGuiCol_ScrollbarBg] = panel;
    c[ImGuiCol_ScrollbarGrab] = border;
    c[ImGuiCol_ScrollbarGrabHovered] = muted;
    c[ImGuiCol_ScrollbarGrabActive] = accent;
    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_CheckboxSelectedBg] = selection;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accentHover;
    c[ImGuiCol_Button] = raised;
    c[ImGuiCol_ButtonHovered] = hover;
    c[ImGuiCol_ButtonActive] = selection;
    c[ImGuiCol_Header] = raised;
    c[ImGuiCol_HeaderHovered] = hover;
    c[ImGuiCol_HeaderActive] = selection;
    c[ImGuiCol_Separator] = border;
    c[ImGuiCol_SeparatorHovered] = accent;
    c[ImGuiCol_SeparatorActive] = accentHover;
    c[ImGuiCol_ResizeGrip] = border;
    c[ImGuiCol_ResizeGripHovered] = accent;
    c[ImGuiCol_ResizeGripActive] = accentHover;
    c[ImGuiCol_InputTextCursor] = accent;
    c[ImGuiCol_Tab] = background;
    c[ImGuiCol_TabHovered] = hover;
    c[ImGuiCol_TabSelected] = panel;
    c[ImGuiCol_TabSelectedOverline] = accent;
    c[ImGuiCol_TabDimmed] = background;
    c[ImGuiCol_TabDimmedSelected] = panel;
    c[ImGuiCol_TabDimmedSelectedOverline] = accent;
    c[ImGuiCol_DockingPreview] = withAlpha(accent, 0.35f);
    c[ImGuiCol_DockingEmptyBg] = background;
    c[ImGuiCol_PlotLines] = muted;
    c[ImGuiCol_PlotLinesHovered] = accentHover;
    c[ImGuiCol_PlotHistogram] = accent;
    c[ImGuiCol_PlotHistogramHovered] = accentHover;
    c[ImGuiCol_TableHeaderBg] = raised;
    c[ImGuiCol_TableBorderStrong] = border;
    c[ImGuiCol_TableBorderLight] = border;
    c[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt] = rgb(255, 255, 255, 0.025f);
    c[ImGuiCol_TextLink] = accent;
    c[ImGuiCol_TextSelectedBg] = selection;
    c[ImGuiCol_TreeLines] = border;
    c[ImGuiCol_DragDropTarget] = accent;
    c[ImGuiCol_DragDropTargetBg] = withAlpha(accent, 0.15f);
    c[ImGuiCol_UnsavedMarker] = folder;
    c[ImGuiCol_NavCursor] = accent;
    c[ImGuiCol_NavWindowingHighlight] = withAlpha(accent, 0.70f);
    c[ImGuiCol_NavWindowingDimBg] = rgb(0, 0, 0, 0.40f);
    c[ImGuiCol_ModalWindowDimBg] = rgb(0, 0, 0, 0.55f);
}

inline void loadFont()
{
    auto& io = ImGui::GetIO();
    if (std::filesystem::exists(fontPath))
        io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath, fontSize);
}

} // namespace GE_Editor::Theme
