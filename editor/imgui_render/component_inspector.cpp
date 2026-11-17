#include "imgui_render.hpp"
#include "EditorTheme.hpp"
#include "ComponentUiTraits.hpp"
#include "ui_state.hpp"

#include <array>
#include <cctype>
#include <string>

namespace GE_Editor
{
namespace
{
using Components = GE::TypeList<GE::TransformComponent, GE::CameraComponent,
    GE::LightComponent, GE::ScriptComponent, GE::MeshComponent>;

constexpr auto componentCount = std::variant_size_v<Components::into<std::variant>>;

bool componentActions(const char* name, float height)
{
    const auto min = ImGui::GetCursorScreenPos();
    if (ImGui::InvisibleButton("Component actions", ImVec2(height, height), ImGuiButtonFlags_EnableNav))
        ImGui::OpenPopup("actions");
    const auto dotColor = ImGui::GetColorU32(ImGui::IsItemHovered() ? ImGuiCol_Text : ImGuiCol_TextDisabled);
    Icons::more(ImVec2(min.x+height*.5f, min.y+height*.5f), Theme::componentMenuDotSpacing, Theme::componentMenuDotRadius, dotColor);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("Component actions");
    bool remove = false;
    if (ImGui::BeginPopup("actions")) {
        ImGui::TextDisabled("%s", name);
        ImGui::Separator();
        remove = ImGui::MenuItem("Remove component");
        ImGui::EndPopup();
    }
    return remove;
}

template<GE::Component T>
bool componentHeader(bool reveal, bool& remove)
{
    using Traits = ComponentUiTraits<T>;
    const auto name = Traits::name;
    ImGui::PushID(name);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Theme::componentHeaderPadding);
    if (reveal) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    const bool open = ImGui::CollapsingHeader("##header", ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen);
    const auto min = ImGui::GetItemRectMin();
    const auto max = ImGui::GetItemRectMax();
    const float height = max.y-min.y;
    if (reveal) ImGui::SetScrollHereY(0.25f);
    ImGui::OpenPopupOnItemClick("actions", ImGuiPopupFlags_MouseButtonRight);
    const float iconX = min.x + ImGui::GetTreeNodeToLabelSpacing();
    Traits::renderIcon(ImVec2(iconX+Theme::componentIconSize*.5f, (min.y+max.y)*.5f),
        Theme::componentIconSize, ImGui::GetColorU32(Theme::componentIconColor), Theme::smallIconStroke);
    const ImVec2 textPos(iconX+Theme::componentIconSize+Theme::componentIconGap,
        (min.y+max.y-ImGui::GetFontSize())*.5f);
    const ImVec4 clip(textPos.x, min.y, max.x-height, max.y);
    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), textPos,
        ImGui::GetColorU32(ImGuiCol_Text), name, nullptr, 0, &clip);
    ImGui::SameLine(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - height);
    remove = componentActions(name, height);
    ImGui::PopStyleVar();
    ImGui::PopID();
    return open && !remove;
}

std::string lower(std::string value)
{
    std::ranges::transform(value, value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

int componentPicker(const std::array<bool, componentCount>& added, ComponentInspectorState& state)
{
    auto& search = state.search;
    if (ImGui::Button("+ Add component", ImVec2(-FLT_MIN, 0))) {
        search.fill(0);
        ImGui::OpenPopup("add_component");
    }
    ImGui::SetNextWindowSize(ImVec2(Theme::componentPickerWidth, 0));
    int chosen = -1;
    if (ImGui::BeginPopup("add_component")) {
        if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##search", "Search components...", search.data(), search.size());
        ImGui::Separator();
        const auto query = lower(search.data());
        int visible = 0;
        int available = 0;
        int index = 0;
        GE::forEachType<Components>([&]<typename T> {
            using Traits = ComponentUiTraits<T>;
            const int i = index++;
            if (lower(Traits::name).find(query) == std::string::npos) return;
            ++visible;
            if (!added.at(i))
                ++available;
            ImGui::PushID(Traits::name);
            ImGui::BeginDisabled(added.at(i));
            const auto rowMin = ImGui::GetCursorScreenPos();
            const float height = ImGui::GetFontSize() + Theme::componentPickerRowPadding*2;
            ImGui::PushStyleColor(ImGuiCol_Header, Theme::selection);
            if (ImGui::Selectable("##component", false, 0, ImVec2(0, height))) chosen = i;
            ImGui::PopStyleColor();
            const auto rowMax = ImGui::GetItemRectMax();
            const float centerY = (rowMin.y+rowMax.y)*.5f;
            const float textY = centerY-ImGui::GetFontSize()*.5f;
            Traits::renderIcon(ImVec2(rowMin.x + Theme::componentIconSize*.5f, centerY),
                Theme::componentIconSize, ImGui::GetColorU32(Theme::componentIconColor), Theme::smallIconStroke);
            ImGui::GetWindowDrawList()->AddText(ImVec2(rowMin.x+Theme::componentIconSize+Theme::componentIconGap, textY),
                ImGui::GetColorU32(ImGuiCol_Text), Traits::name);
            if (added.at(i)) {
                const char* badge = "Added";
                ImGui::GetWindowDrawList()->AddText(ImVec2(rowMax.x-ImGui::CalcTextSize(badge).x-ImGui::GetStyle().FramePadding.x,
                    textY), ImGui::GetColorU32(ImGuiCol_TextDisabled), badge);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("%s%s", Traits::description, added.at(i) ? "\nAlready added to this entity." : "");
            ImGui::EndDisabled();
            ImGui::PopID();
        });
        if (visible == 0)
            ImGui::TextDisabled("No components match your search.");
        else if (available == 0)
            ImGui::TextDisabled("Matching components are already added.");
        if (chosen>=0) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    return chosen;
}
} // namespace

void renderComponentInspector(GE::Entity entity, GE::AssetManager& assets, const GE::ScriptLibrary* scripts, ComponentInspectorState& state)
{
    // Scope expansion and popup state to the entity, so selection changes do not affect another entity's cards.
    ImGui::PushID(uuids::to_string(entity.entityId).c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Theme::componentSpacing);
    auto& previousEntity = state.previousEntity;
    auto& reveal = state.reveal;
    if (previousEntity != entity) { reveal = -1; previousEntity = entity; }
    std::array<bool, componentCount> added{};
    int index = 0;
    GE::forEachType<Components>([&]<typename ComponentT> {
        const int i = index++;
        if (!entity.has<ComponentT>()) return;
        bool remove = false;
        if (componentHeader<ComponentT>(reveal==i, remove))
            componentEditWidget<ComponentT>(entity, assets, scripts);
        if (remove) entity.remove<ComponentT>();
        added.at(i) = entity.has<ComponentT>();
    });
    reveal = -1;
    ImGui::Separator();
    const int chosen = componentPicker(added, state);
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
