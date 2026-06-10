#pragma once

#include "Game-Engine/ScriptLibrary.hpp"
#include "Game-Engine/Input.hpp"

#include <Game-Engine/Components.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/ScriptLibrary.hpp>
#include <Game-Engine/Scene.hpp>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <array>

#include <glm/glm.hpp>

#include <imgui.h>

namespace GE_Editor
{

template<size_t S>
void strBuff(std::string_view str, auto&& render)
{
    std::array<char, S> buffer{};
    const size_t size = std::min(str.size(), buffer.size() - 1);
    std::copy_n(str.data(), size, buffer.data()); // NOLINT(bugprone-suspicious-stringview-data-usage)
    render(buffer.data());
}

template<size_t S>
void floatBuff(glm::vec<S, float> vec, auto&& render)
{
    std::array<float, S> buffer{};
    for (size_t i = 0; i < S; i++)
        buffer[i] = vec[i];
    render(buffer.data());
}

template<GE::Component T>
void componentEditWidget(GE::Entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*) { static_assert(false, "not implemented"); }

template<>
void componentEditWidget<GE::NameComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*);

template<>
void componentEditWidget<GE::TransformComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*);

template<>
void componentEditWidget<GE::CameraComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*);

template<>
void componentEditWidget<GE::LightComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*);

template<>
void componentEditWidget<GE::ScriptComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*);

template<>
void componentEditWidget<GE::MeshComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*);

template<typename Fn>
bool collapsingHeaderWithActionButton(const char* title, const char* buttonLabel, std::string_view buttonIdSuffix, const Fn& fn)
{
    const bool isOpen = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen);
    const ImGuiStyle& style = ImGui::GetStyle();
    const float buttonWidth = ImGui::CalcTextSize(buttonLabel).x + style.FramePadding.x * 2.0f;
    const float buttonX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonWidth;
    ImGui::SameLine(buttonX);

    std::string buttonId = std::string(buttonLabel) + "##" + std::string(buttonIdSuffix);
    if (ImGui::SmallButton(buttonId.c_str()))
    {
        fn();
        return false;
    }
    return isOpen;
}

struct EditableProjectProperties
{
    std::string& projectName;                                // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    std::optional<std::filesystem::path>& projectPath;       // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    std::optional<std::filesystem::path>& scriptLibPath;     // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    std::vector<std::pair<std::string, GE::VInput>>& inputs; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

bool renderPropertiesWindow(EditableProjectProperties&);

void renderAssetManagerDebugWindow(GE::AssetManager&);

using MenuItem = std::pair<std::string_view, std::function<void()>>;

template<typename T>
concept MenuItemRange = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, MenuItem>;

void menuFromPath(MenuItemRange auto&& items, int offset = 0)
{
    for (auto it = items.begin(); it != items.end();)
    {
        const std::string_view rest = it->first.substr(offset);
        const size_t slash = rest.find('/');
        const std::string_view name = rest.substr(0, slash);

        if (slash == std::string_view::npos)
        {
            strBuff<128>(name, [&](const char* cstr) {
                ImGui::BeginDisabled(!it->second);
                if (ImGui::MenuItem(cstr) && it->second)
                    it->second();
                ImGui::EndDisabled();
            });
            ++it;
            continue;
        }

        const size_t childOffset = offset + slash + 1;
        auto childEnd = std::next(it);
        while (childEnd != items.end() && childEnd->first.size() > childOffset && childEnd->first.substr(offset, slash) == name && childEnd->first[offset + slash] == '/')
            ++childEnd;

        strBuff<128>(name, [&](const char* cstr) {
            if (ImGui::BeginMenu(cstr))
            {
                menuFromPath(std::ranges::subrange(it, childEnd), childOffset);
                ImGui::EndMenu();
            }
        });

        it = childEnd;
    }
}

void renderSceneGraph(GE::Scene& editedScene, std::optional<GE::Entity>& selectedEntity, GE::AssetManager& assetManager);

template<typename T>
void tileGrid(const std::ranges::range auto& items, const std::function<void(const T&)>& fn) requires std::same_as<std::ranges::range_value_t<std::remove_cvref_t<decltype(items)>>, T>
{
    for (const auto& item : items)
    {
        float posA = ImGui::GetCursorScreenPos().x;
        fn(item);
        ImGui::SameLine();
        float posB = ImGui::GetCursorScreenPos().x;
        float tileSize = posB - posA;
        if (tileSize > ImGui::GetContentRegionAvail().x)
            ImGui::NewLine();
    }
}
}
