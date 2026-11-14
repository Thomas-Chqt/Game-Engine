#include "imgui_render.hpp"
#include "EditorTheme.hpp"

#include <array>
#include <cctype>
#include <string>
#include <vector>

namespace GE_Editor
{
namespace
{
using Components = GE::TypeList<GE::TransformComponent, GE::CameraComponent,
    GE::LightComponent, GE::ScriptComponent, GE::MeshComponent>;

struct ComponentInfo { const char* name; const char* description; };
constexpr std::array<ComponentInfo, 5> componentInfo = {{
    {"Transform", "Position, rotation, and scale in the scene."},
    {"Camera", "View the scene through a perspective camera."},
    {"Light", "Illuminate the scene with a point or directional light."},
    {"Script", "Attach gameplay behavior from the project's script library."},
    {"Mesh", "Display a mesh from the project's assets."},
}};

// Match the toolbar's 24-unit design grid; keep fine details out of small icons.
void componentIcon(int index, ImVec2 center)
{
    auto* draw = ImGui::GetWindowDrawList();
    const float size = Theme::componentIconSize;
    const float stroke = Theme::smallIconStroke;
    const auto color = ImGui::GetColorU32(Theme::componentIconColor);
    const auto p = [&](float x, float y) { return ImVec2(center.x+(x-12)*size/24, center.y+(y-12)*size/24); };
    const auto line = [&](float x, float y, float a, float b) { draw->AddLine(p(x,y), p(a,b), color, stroke); };
    switch (index) {
        case 0: // Transform: three axes, matching the Local space icon.
            line(7,17,7,5); line(7,17,19,17); line(7,17,16,8);
            draw->AddTriangleFilled(p(7,2), p(4.5f,6), p(9.5f,6), color);
            draw->AddTriangleFilled(p(22,17), p(18,14.5f), p(18,19.5f), color);
            draw->AddTriangleFilled(p(18,6), p(13.5f,7), p(17,10.5f), color);
            break;
        case 1: // Camera: side-view body and projecting lens.
            draw->AddRect(p(2,6), p(15,18), color, size/24, 0, stroke);
            draw->PathLineTo(p(15,10)); draw->PathLineTo(p(22,6));
            draw->PathLineTo(p(22,18)); draw->PathLineTo(p(15,14));
            draw->PathStroke(color, ImDrawFlags_Closed, stroke);
            break;
        case 2: // Light: bulb silhouette and base, rather than crowded sun rays.
            draw->PathLineTo(p(9,17));
            draw->PathBezierCubicCurveTo(p(9,14), p(5,14), p(5,9));
            draw->PathBezierCubicCurveTo(p(5,5), p(8,2), p(12,2));
            draw->PathBezierCubicCurveTo(p(16,2), p(19,5), p(19,9));
            draw->PathBezierCubicCurveTo(p(19,14), p(15,14), p(15,17));
            draw->PathStroke(color, ImDrawFlags_Closed, stroke);
            line(10,20,14,20);
            break;
        case 3: // Script: code brackets and slash.
            draw->PathLineTo(p(7,7)); draw->PathLineTo(p(3,12)); draw->PathLineTo(p(7,17));
            draw->PathStroke(color, 0, stroke);
            draw->PathLineTo(p(17,7)); draw->PathLineTo(p(21,12)); draw->PathLineTo(p(17,17));
            draw->PathStroke(color, 0, stroke);
            line(14,4,10,20);
            break;
        case 4: // Mesh: a balanced isometric cube with three readable faces.
            draw->PathLineTo(p(12,2)); draw->PathLineTo(p(21,7)); draw->PathLineTo(p(21,17));
            draw->PathLineTo(p(12,22)); draw->PathLineTo(p(3,17)); draw->PathLineTo(p(3,7));
            draw->PathStroke(color, ImDrawFlags_Closed, stroke);
            line(3,7,12,12); line(21,7,12,12); line(12,12,12,22);
            break;
    }
}

bool componentActions(int index, float height)
{
    const auto min = ImGui::GetCursorScreenPos();
    if (ImGui::InvisibleButton("Component actions", ImVec2(height, height), ImGuiButtonFlags_EnableNav))
        ImGui::OpenPopup("actions");
    const auto dotColor = ImGui::GetColorU32(ImGui::IsItemHovered() ? ImGuiCol_Text : ImGuiCol_TextDisabled);
    for (int dot=-1; dot<=1; ++dot)
        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(min.x+height*.5f,
            min.y+height*.5f+dot*Theme::componentMenuDotSpacing), Theme::componentMenuDotRadius, dotColor);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) ImGui::SetTooltip("Component actions");
    bool remove = false;
    if (ImGui::BeginPopup("actions")) {
        ImGui::TextDisabled("%s", componentInfo[index].name);
        ImGui::Separator();
        remove = ImGui::MenuItem("Remove component");
        ImGui::EndPopup();
    }
    return remove;
}

bool componentHeader(int index, bool reveal, bool& remove)
{
    ImGui::PushID(index);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Theme::componentHeaderPadding);
    if (reveal) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    const bool open = ImGui::CollapsingHeader("##header", ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen);
    const auto min = ImGui::GetItemRectMin();
    const auto max = ImGui::GetItemRectMax();
    const float height = max.y-min.y;
    if (reveal) ImGui::SetScrollHereY(0.25f);
    ImGui::OpenPopupOnItemClick("actions", ImGuiPopupFlags_MouseButtonRight);
    const float iconX = min.x + ImGui::GetTreeNodeToLabelSpacing();
    componentIcon(index, ImVec2(iconX+Theme::componentIconSize*.5f, (min.y+max.y)*.5f));
    const ImVec2 textPos(iconX+Theme::componentIconSize+Theme::componentIconGap,
        (min.y+max.y-ImGui::GetFontSize())*.5f);
    const ImVec4 clip(textPos.x, min.y, max.x-height, max.y);
    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), textPos,
        ImGui::GetColorU32(ImGuiCol_Text), componentInfo[index].name, nullptr, 0, &clip);
    ImGui::SameLine(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - height);
    remove = componentActions(index, height);
    ImGui::PopStyleVar();
    ImGui::PopID();
    return open && !remove;
}

std::string lower(std::string value)
{
    std::ranges::transform(value, value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

int componentPicker(const std::array<bool, 5>& added)
{
    static std::array<char, 96> search{};
    static int highlighted = -1;
    if (ImGui::Button("+ Add component", ImVec2(-FLT_MIN, 0))) {
        search.fill(0);
        highlighted = -1;
        ImGui::OpenPopup("add_component");
    }
    ImGui::SetNextWindowSize(ImVec2(Theme::componentPickerWidth, 0));
    int chosen = -1;
    if (ImGui::BeginPopup("add_component")) {
        if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
        ImGui::SetNextItemWidth(-FLT_MIN);
        const bool changed = ImGui::InputTextWithHint("##search", "Search components...", search.data(), search.size());
        ImGui::Separator();
        const auto query = lower(search.data());
        std::vector<int> visible;
        std::vector<int> available;
        for (int i=0; i<static_cast<int>(componentInfo.size()); ++i) {
            if (lower(componentInfo[i].name).find(query) == std::string::npos) continue;
            visible.push_back(i);
            if (!added[i]) available.push_back(i);
        }
        if (changed || std::ranges::find(available, highlighted) == available.end())
            highlighted = available.empty() ? -1 : available.front();
        if (!available.empty()) {
            const auto current = std::ranges::find(available, highlighted) - available.begin();
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) highlighted = available[(current+1)%available.size()];
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) highlighted = available[(current+available.size()-1)%available.size()];
            if (ImGui::IsKeyPressed(ImGuiKey_Enter)) chosen = highlighted;
        }
        for (int i : visible) {
            ImGui::PushID(i);
            ImGui::BeginDisabled(added[i]);
            const auto rowMin = ImGui::GetCursorScreenPos();
            const float height = ImGui::GetFontSize() + Theme::componentPickerRowPadding*2;
            ImGui::PushStyleColor(ImGuiCol_Header, Theme::selection);
            if (ImGui::Selectable("##component", i==highlighted, 0, ImVec2(0, height))) chosen = i;
            ImGui::PopStyleColor();
            const auto rowMax = ImGui::GetItemRectMax();
            const float centerY = (rowMin.y+rowMax.y)*.5f;
            const float textY = centerY-ImGui::GetFontSize()*.5f;
            componentIcon(i, ImVec2(rowMin.x + Theme::componentIconSize*.5f, centerY));
            ImGui::GetWindowDrawList()->AddText(ImVec2(rowMin.x+Theme::componentIconSize+Theme::componentIconGap, textY),
                ImGui::GetColorU32(ImGuiCol_Text), componentInfo[i].name);
            if (added[i]) {
                const char* badge = "Added";
                ImGui::GetWindowDrawList()->AddText(ImVec2(rowMax.x-ImGui::CalcTextSize(badge).x-ImGui::GetStyle().FramePadding.x,
                    textY), ImGui::GetColorU32(ImGuiCol_TextDisabled), badge);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("%s%s", componentInfo[i].description, added[i] ? "\nAlready added to this entity." : "");
            ImGui::EndDisabled();
            ImGui::PopID();
        }
        if (visible.empty()) ImGui::TextDisabled("No components match your search.");
        else if (available.empty()) ImGui::TextDisabled("Matching components are already added.");
        ImGui::Separator();
        ImGui::TextDisabled("Arrow keys to select - Enter to add");
        if (chosen>=0 || ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    return chosen;
}
} // namespace

void renderComponentInspector(GE::Entity entity, GE::AssetManager& assets, const GE::ScriptLibrary* scripts)
{
    // Scope expansion and popup state to the entity, so selection changes do not affect another entity's cards.
    ImGui::PushID(uuids::to_string(entity.entityId).c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Theme::componentSpacing);
    static std::optional<GE::Entity> previousEntity;
    static int reveal = -1;
    if (previousEntity != entity) { reveal = -1; previousEntity = entity; }
    std::array<bool, 5> added{};
    int index = 0;
    GE::forEachType<Components>([&]<typename ComponentT> {
        const int i = index++;
        if (!entity.has<ComponentT>()) return;
        bool remove = false;
        if (componentHeader(i, reveal==i, remove))
            componentEditWidget<ComponentT>(entity, assets, scripts);
        if (remove) entity.remove<ComponentT>();
        added[i] = entity.has<ComponentT>();
    });
    reveal = -1;
    ImGui::Separator();
    const int chosen = componentPicker(added);
    index = 0;
    GE::forEachType<Components>([&]<typename ComponentT> {
        if (index++ != chosen) return;
        entity.emplace<ComponentT>();
        if constexpr (std::same_as<ComponentT, GE::MeshComponent>) assets.loadAsset(entity.get<GE::MeshComponent>().id);
        reveal = chosen;
    });
    ImGui::PopStyleVar();
    ImGui::PopID();
}
} // namespace GE_Editor
