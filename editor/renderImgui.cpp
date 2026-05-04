/*
 * ---------------------------------------------------
 * renderImgui.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Editor.hpp"

#include "Game-Engine/ECSView.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/InputFwd.hpp"
#include "Game-Engine/RawInput.hpp"
#include "Game-Engine/Scene.hpp"

#include "imgui.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <numbers>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>

constexpr float TILE_SIZE = 60.0f;

namespace
{

using MenuItem = std::pair<std::string_view, std::function<void()>>;

template<typename T>
concept MenuItemRange = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, MenuItem>;

using EditableEntityComponents = GE::TypeList< GE::TransformComponent, GE::CameraComponent, GE::LightComponent, GE::ScriptComponent, GE::MeshComponent >;

template<typename T> struct EditableEntityComponentTraits;
template<> struct EditableEntityComponentTraits<GE::TransformComponent> { static constexpr const char* label = "Transform component"; };
template<> struct EditableEntityComponentTraits<GE::CameraComponent>    { static constexpr const char* label = "Camera component";    };
template<> struct EditableEntityComponentTraits<GE::LightComponent>     { static constexpr const char* label = "Light component";     };
template<> struct EditableEntityComponentTraits<GE::ScriptComponent>    { static constexpr const char* label = "Script component";    };
template<> struct EditableEntityComponentTraits<GE::MeshComponent>      { static constexpr const char* label = "Mesh component";      };

constexpr const char* NO_INPUT_MAPPER_LABEL = "None";

template<typename T> struct EditableInputTraits;
template<> struct EditableInputTraits<GE::ActionInput>  { static constexpr const char* label = "Action"; };
template<> struct EditableInputTraits<GE::StateInput>   { static constexpr const char* label = "State";  };
template<> struct EditableInputTraits<GE::RangeInput>   { static constexpr const char* label = "Range";  };
template<> struct EditableInputTraits<GE::Range2DInput> { static constexpr const char* label = "Range2D"; };

template<typename T> struct EditableRawInputTraits;
template<> struct EditableRawInputTraits<GE::KeyboardButton> { static constexpr const char* label = "Keyboard button"; };

void keyboardButtonCombo(const char* label, GE::KeyboardButton& button)
{
    if (ImGui::BeginCombo(label, GE::keyboardButtonName(button).data()))
    {
        for (GE::KeyboardButton candidate : GE::KEYBOARD_BUTTONS)
        {
            const bool isSelected = (candidate == button);
            if (ImGui::Selectable(GE::keyboardButtonName(candidate).data(), isSelected))
                button = candidate;
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

template<typename Fn>
void propertyRow(const char* label, Fn&& fn)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    fn();
}

bool beginPropertyTable(const char* id)
{
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV))
        return false;

    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 130.0f);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

template<typename Fn>
bool collapsingHeaderWithActionButton(const char* title, const char* buttonLabel, std::string_view buttonIdSuffix, Fn&& fn)
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

template<typename T>
void componentEditWidget(GE::Entity&, GE::Scene&)
{
    static_assert(false, "not implemented");
}

template<>
void componentEditWidget<GE::NameComponent>(GE::Entity& entity, GE::Scene&)
{
    GE::NameComponent& nameComponent = entity.get<GE::NameComponent>();
    char buff[32];
    std::strncpy(buff, nameComponent.name.c_str(), sizeof(buff));
    buff[31] = '\0'; // If count is reached before the entire string src was copied, the resulting character array is not null-terminated.
    ImGui::InputText("Name##NameComponent_name", buff, sizeof(buff));
    nameComponent.name = std::string(buff);
}

template<>
void componentEditWidget<GE::TransformComponent>(GE::Entity& entity, GE::Scene&)
{
    GE::TransformComponent& transform = entity.get<GE::TransformComponent>();
    ImGui::DragFloat3("position##TransformComponent_position", (float*)&transform.position, 0.01f, -1000.0f, 1000.0f);
    ImGui::DragFloat3("rotation##TransformComponent_rotation", (float*)&transform.rotation, 0.01f, -2*std::numbers::pi_v<float>, 2*std::numbers::pi_v<float>);
    ImGui::DragFloat3("scale##TransformComponent_scale",       (float*)&transform.scale,    0.01f, 0.0f, 10.0f);
}

template<>
void componentEditWidget<GE::CameraComponent>(GE::Entity& entity, GE::Scene&)
{
    GE::CameraComponent& cameraComponent = entity.get<GE::CameraComponent>();

    ImGui::DragFloat("fov##CameraComponent_fov",     &cameraComponent.fov,   0.01f,  -2*std::numbers::pi_v<float>, 2*std::numbers::pi_v<float>);
    ImGui::DragFloat("zFar##CameraComponent_zFar",   &cameraComponent.zFar,  0.01f, 0.001f, 10000.0f);
    ImGui::DragFloat("zNear##CameraComponent_zNear", &cameraComponent.zNear, 0.01f, 0.001f, 10000.0f);
}

template<>
void componentEditWidget<GE::LightComponent>(GE::Entity& entity, GE::Scene&)
{
    auto typeToStr = [](const GE::LightComponent::Type& type){
        switch (type) {
            case GE::LightComponent::Type::directional: return "directional";
            case GE::LightComponent::Type::point: return "point";
        }
        std::unreachable();
    };

    GE::LightComponent& lightComponent = entity.get<GE::LightComponent>();
    if (ImGui::BeginCombo("Type##LightComponent_type", typeToStr(lightComponent.type)))
    {
        for (auto& type : {GE::LightComponent::Type::directional, GE::LightComponent::Type::point})
        {
            const bool is_selected = (lightComponent.type == type);
            if (ImGui::Selectable(typeToStr(type), is_selected))
                lightComponent.type = type;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::ColorEdit3("color##LightComponent_color", (float*)&lightComponent.color);
    ImGui::DragFloat("intentsity##LightComponent_intensity", &lightComponent.intentsity, 0.01, 0.0f, 1.0f);
    if (lightComponent.type == GE::LightComponent::Type::point)
        ImGui::DragFloat("attenuation##LightComponent_attenuation", &lightComponent.attenuation, 0.01, 0.0f, 1.0f);
}

void scriptComponentEditWidget(GE::Entity& entity, GE::Scene&, const GE::ScriptLibrary& scriptLibrary)
{
    GE::ScriptComponent& scriptComponent = entity.get<GE::ScriptComponent>();
    const char* previewValue = scriptComponent.name.empty() ? "none" : scriptComponent.name.c_str();
    if (ImGui::BeginCombo("Script##ScriptComponent_name", previewValue))
    {
        for (const std::string& scriptName : scriptLibrary.listScriptNames())
        {
            const bool isSelected = scriptComponent.name == scriptName;
            if (ImGui::Selectable(scriptName.c_str(), isSelected))
            {
                scriptComponent.name = scriptName;
                scriptComponent.parameters.clear();
                for (const std::string& parameterName : scriptLibrary.listScriptParameterNames(scriptName))
                {
                    auto [parameterIt, inserted] = scriptComponent.parameters.try_emplace(parameterName, scriptLibrary.getScriptDefaultParameterValue(scriptName, parameterName));
                    assert(inserted);
                }
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    for (auto& [name, value] : scriptComponent.parameters)
    {
        std::visit([&](auto& typedValue)
        {
            using T = std::remove_cvref_t<decltype(typedValue)>;
            if constexpr (std::is_same_v<T, bool>)
                ImGui::Checkbox(name.c_str(), &typedValue);
            else if constexpr (std::is_same_v<T, int64_t>)
            {
                int64_t value64 = typedValue;
                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &value64))
                    typedValue = value64;
            }
            else if constexpr (std::is_same_v<T, float>)
                ImGui::DragFloat(name.c_str(), &typedValue, 0.01f);
            else if constexpr (std::is_same_v<T, glm::vec2>)
                ImGui::DragFloat2(name.c_str(), &typedValue.x, 0.01f);
            else if constexpr (std::is_same_v<T, glm::vec3>)
                ImGui::DragFloat3(name.c_str(), &typedValue.x, 0.01f);
            else if constexpr (std::is_same_v<T, std::string>)
            {
                char buffer[256];
                std::strncpy(buffer, typedValue.c_str(), sizeof(buffer));
                buffer[sizeof(buffer) - 1] = '\0';
                if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
                    typedValue = buffer;
            }
        }, value);
    }
}

template<>
void componentEditWidget<GE::MeshComponent>(GE::Entity& entity, GE::Scene& scene)
{
    GE::MeshComponent& meshComponent = entity.get<GE::MeshComponent>();

    std::string currentMeshStem;
    if (meshComponent.id == GE::BUILT_IN_CUBE_ASSET_ID)
        currentMeshStem = "built_in_cube";
    else
    {
        const auto& registredAssets = scene.assetManagerView().registredAssets();
        const auto assetIt = std::ranges::find_if(registredAssets, [&](const auto& asset) { return asset.second == meshComponent.id; });
        if (assetIt != registredAssets.end())
            currentMeshStem = std::visit([](auto& path){return path.path.stem().string();}, assetIt->first);
    }

    if (ImGui::BeginCombo("Type##LightComponent_type", currentMeshStem.c_str()))
    {
        if (ImGui::Selectable("built_in_cube", meshComponent.id == GE::BUILT_IN_CUBE_ASSET_ID))
            meshComponent.id = GE::BUILT_IN_CUBE_ASSET_ID;
        if (meshComponent.id == GE::BUILT_IN_CUBE_ASSET_ID)
            ImGui::SetItemDefaultFocus();

        for (auto& [vAssetPath, id] : scene.assetManagerView().registredAssets())
        {
            const bool is_selected = (id == meshComponent.id);
            std::string meshStem = std::visit([](auto& path){ return path.path.stem().string(); }, vAssetPath);
            if (ImGui::Selectable(meshStem.c_str(), is_selected))
                meshComponent.id = id;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_dnd")) {
            GE::Entity& droped = *(GE::Entity*)payload->Data;
            std::string_view path = std::string_view((const char*)payload->Data, (size_t)payload->DataSize);
            GE::AssetID id = scene.assetManagerView().registerAsset<GE::Mesh>(std::filesystem::path(path));
            meshComponent.id = id;
        }
        ImGui::EndDragDropTarget();
    }
}

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

void menuFromPath(MenuItemRange auto&& items, int offset = 0)
{
    auto label = [](std::string_view str, auto&& render)
    {
        std::array<char, 128> buffer{};
        const size_t size = std::min(str.size(), buffer.size() - 1);
        std::copy_n(str.data(), size, buffer.data());
        render(buffer.data());
    };

    for (auto it = items.begin(); it != items.end();)
    {
        const std::string_view rest = it->first.substr(offset);
        const size_t slash = rest.find('/');
        const std::string_view name = rest.substr(0, slash);

        if (slash == std::string_view::npos)
        {
            label(name, [&](const char* cstr) {
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

        label(name, [&](const char* cstr) {
            if (ImGui::BeginMenu(cstr))
            {
                menuFromPath(std::ranges::subrange(it, childEnd), childOffset);
                ImGui::EndMenu();
            }
        });

        it = childEnd;
    }
}

void sceneGrapRow(GE::Entity& entity, GE::Entity& selectedEntity)
{
    bool node_open = false;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

        if (selectedEntity == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (entity.children().size() > 0)
            node_open = ImGui::TreeNodeEx((void*)entity.entityId, flags, "%s", entity.name().c_str());
        else
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx((void*)entity.entityId, flags, "%s", entity.name().c_str());
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("dnd_entity", &entity, sizeof(GE::const_Entity));
            ImGui::Text("%s", entity.name().c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_entity")) {
                assert(payload->DataSize == sizeof(GE::Entity));
                GE::Entity& droped = *(GE::Entity*)payload->Data;
                if (droped.isParentOf(entity) == false) {
                    if(auto parent = droped.parent())
                        parent->removeChild(droped);
                    entity.addChild(droped);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked())
            selectedEntity = entity;

        if (ImGui::BeginPopupContextItem())
        {
            menuFromPath(std::to_array<MenuItem>({
                {"Delete", [&] {
                    if (selectedEntity == entity)
                        selectedEntity = GE::Entity{};
                    entity.destroy();
                }}
            }));
            ImGui::EndPopup();
            if (entity.world == nullptr || entity.entityId == INVALID_ENTITY_ID)
                return;
        }

        if (node_open && entity.children().size() > 0)
        {
            for (auto curr = entity.firstChild(); curr; curr = curr->nextChild() )
                sceneGrapRow(*curr, selectedEntity);
            ImGui::TreePop();
        }
}

}

namespace GE_Editor
{

void Editor::renderImgui()
{
    static bool projectPropertiesOpen = false;
    static std::filesystem::path resourceBrowserSubDir;

    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    if (ImGui::BeginMainMenuBar())
    {
        menuFromPath(std::to_array<MenuItem>({
            {"file/new",                  {}},

            {"file/open",                 {}},

            {"file/save",                 !m_projectFilePath.empty() ? [this]() { saveProject(); }
                                                                     : std::function<void()>()},

            {"project/properties",        []() { projectPropertiesOpen = true; }},

            {"project/reload script lib", std::filesystem::exists(m_project.scriptLib()) && !m_game.has_value() ? [this]() { reloadScriptLib(); }
                                                                                                                : std::function<void()>()},

            {"project/run",               m_game.has_value() ? std::function<void()>()
                                                             : [this]() { startGame(); }},

            {"project/stop",              m_game.has_value() ? [this]() { stopGame(); }
                                                             : std::function<void()>()},

        }));
    }
    ImGui::EndMainMenuBar();

    if (ImGui::Begin("viewport"))
    {
        static std::variant<std::string, uint64_t> textureIdPlaceholder = std::string("viewportBackBuffer");

        ImVec2 contentRegionAvai = ImGui::GetContentRegionAvail();
        uint32_t newWidth = contentRegionAvai.x <= 0 ? 1 : contentRegionAvai.x;
        uint32_t newHeight = contentRegionAvai.y <= 0 ? 1 : contentRegionAvai.y;

        ImGui::Image(&textureIdPlaceholder, contentRegionAvai);

        if (newWidth != m_viewportSize.first || newHeight != m_viewportSize.second) {
            m_viewportSize = {newWidth, newHeight};
            rebuildFrameGraph();
        }
    }
    ImGui::End();

    if (ImGui::Begin("Scene graph"))
    {
        if (ImGui::BeginChild("scene_graph_child"))
        {
            for (GE::Entity entity : m_editedScene.second.ecsWorld()
                                        | GE::ECSView<GE::NameComponent>()
                                        | std::views::transform([&](auto id){ return GE::Entity{&m_editedScene.second.ecsWorld(), id}; })
                                        | std::ranges::to<std::vector>())
            {
                if (entity.parent().has_value() == false)
                    sceneGrapRow(entity, m_selectedEntity);
            }
            if (ImGui::BeginPopupContextWindow("scene_graph_context", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                menuFromPath(std::to_array<MenuItem>({
                    {"Add/Cube", [this] {
                        m_selectedEntity = m_editedScene.second.newEntity("cube");
                        m_selectedEntity.emplace<GE::TransformComponent>();
                        m_selectedEntity.emplace<GE::MeshComponent>(GE::BUILT_IN_CUBE_ASSET_ID);
                    }},
                    {"Add/Light", [this] {
                        m_selectedEntity = m_editedScene.second.newEntity("light");
                        m_selectedEntity.emplace<GE::TransformComponent>();
                        m_selectedEntity.emplace<GE::LightComponent>();
                    }},
                    {"Add/Camera", [this] {
                        m_selectedEntity = m_editedScene.second.newEntity("camera");
                        m_selectedEntity.emplace<GE::TransformComponent>();
                        m_selectedEntity.emplace<GE::CameraComponent>();
                        if (m_editedScene.second.activeCamera().entityId == INVALID_ENTITY_ID)
                            m_editedScene.second.setActiveCamera(m_selectedEntity);
                    }}
                }));
                ImGui::EndPopup();
            }
            ImGui::EndChild();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_entity")) {
                assert(payload->DataSize == sizeof(GE::Entity));
                GE::Entity& droped = *(GE::Entity*)payload->Data;
                if(auto parent = droped.parent())
                    parent->removeChild(droped);
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();

    if (ImGui::Begin("Scenes"))
    {
        tileGrid(m_project.scenes() | std::views::values, std::function([](const GE::Scene::Descriptor& scene){
            ImGui::BeginGroup();
            {
                ImGui::Button(scene.name.c_str(), ImVec2(TILE_SIZE, TILE_SIZE));
                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("scene_dnd", scene.name.c_str(), scene.name.size());
                    ImGui::Text("%s", scene.name.c_str());
                    ImGui::EndDragDropSource();
                }
                ImVec2 textMin = ImGui::GetCursorScreenPos();
                ImVec2 textSize = {TILE_SIZE, ImGui::GetTextLineHeightWithSpacing()};
                ImVec4 clipRect(textMin.x, textMin.y, textMin.x + textSize.x, textMin.y + textSize.y);
                ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), textMin, ImGui::GetColorU32(ImGuiCol_Text), scene.name.c_str(), nullptr, 0.0f, &clipRect);
                ImGui::Dummy(textSize);
            }
            ImGui::EndGroup();
        }));
    }
    ImGui::End();

    if (ImGui::Begin("Resources") && std::filesystem::exists(m_project.resourceDir()))
    {
        ImGui::BeginDisabled(resourceBrowserSubDir.empty());
        if (ImGui::Button("<"))
            resourceBrowserSubDir = resourceBrowserSubDir.parent_path();
        ImGui::EndDisabled();
        ImGui::SameLine();
        const std::string resourcePath = (m_project.resourceDir().filename() / resourceBrowserSubDir).string();
        ImGui::TextUnformatted(resourcePath.c_str());
        ImGui::BeginChild("ResourcesChild");
        tileGrid(std::filesystem::directory_iterator(m_project.resourceDir() / resourceBrowserSubDir), std::function([](const std::filesystem::directory_entry& entry){
            ImGui::BeginGroup();
            {
                const std::string entryPath = entry.path().string();
                const std::string entryName = entry.path().filename().string();
                if (entry.is_directory()) {
                    ImGui::Button(std::format("DIR##{}", entryPath).c_str(), ImVec2(TILE_SIZE, TILE_SIZE));
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        resourceBrowserSubDir /= entry.path().filename();
                }
                else {
                    ImGui::Button(std::format("FILE##{}", entryPath).c_str(), ImVec2(TILE_SIZE, TILE_SIZE));
                    if (ImGui::BeginDragDropSource())
                    {
                        ImGui::SetDragDropPayload("resource_dnd", entryPath.c_str(), entryPath.size() + 1);
                        ImGui::Text("%s", entryName.c_str());
                        ImGui::EndDragDropSource();
                    }
                }
                ImVec2 textMin = ImGui::GetCursorScreenPos();
                ImVec2 textSize = {TILE_SIZE, ImGui::GetTextLineHeightWithSpacing()};
                ImVec4 clipRect(textMin.x, textMin.y, textMin.x + textSize.x, textMin.y + textSize.y);
                ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), textMin, ImGui::GetColorU32(ImGuiCol_Text), entryName.c_str(), nullptr, 0.0f, &clipRect);
                ImGui::Dummy(textSize);
            }
            ImGui::EndGroup();
        }));
        ImGui::EndChild();
    }
    ImGui::End();

    if (ImGui::Begin("Entity inspector"))
    {
        ImGui::PushItemWidth(-80);
        if (m_selectedEntity.world == nullptr || m_selectedEntity.entityId == INVALID_ENTITY_ID)
            ImGui::Text("No entity selected");
        else
        {
            componentEditWidget<GE::NameComponent>(m_selectedEntity, m_editedScene.second);
            GE::forEachType<EditableEntityComponents>([&]<typename ComponentT>() {
                if (m_selectedEntity.has<ComponentT>() && collapsingHeaderWithActionButton(EditableEntityComponentTraits<ComponentT>::label, "Remove", EditableEntityComponentTraits<ComponentT>::label, [&] {
                    m_selectedEntity.remove<ComponentT>();
                }))
                {
                    if constexpr (std::is_same_v<ComponentT, GE::ScriptComponent>)
                    {
                        if (m_scriptLibrary.has_value())
                            scriptComponentEditWidget(m_selectedEntity, m_editedScene.second, *m_scriptLibrary);
                        else
                            ImGui::TextDisabled("No script library loaded");
                    }
                    else
                        componentEditWidget<ComponentT>(m_selectedEntity, m_editedScene.second);
                }
            });

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Add component", ImVec2(-FLT_MIN, 0.0f)))
                ImGui::OpenPopup("add_component_popup");
            if (ImGui::BeginPopup("add_component_popup"))
            {
                GE::forEachType<EditableEntityComponents>([&]<typename ComponentT>() {
                    if (m_selectedEntity.has<ComponentT>() == false && ImGui::Selectable(EditableEntityComponentTraits<ComponentT>::label))
                        m_selectedEntity.emplace<ComponentT>();
                });

                ImGui::EndPopup();
            }

        }
        ImGui::PopItemWidth();
    }
    ImGui::End();

    if (projectPropertiesOpen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 windowSize(viewport->Size.x * 0.42f, viewport->Size.y * 0.70f);
        ImGui::SetNextWindowPos(
            ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.5f),
            ImGuiCond_Appearing,
            ImVec2(0.5f, 0.5f)
        );
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);

        if (ImGui::Begin("Project properties", &projectPropertiesOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
        {
            ImGui::Spacing();

            if (beginPropertyTable("##project_settings"))
            {
                propertyRow("Project name", [&] {
                    char projectName[256];
                    std::strncpy(projectName, m_project.name().c_str(), sizeof(projectName));
                    projectName[255] = '\0';
                    if (ImGui::InputText("##project_name", projectName, sizeof(projectName)))
                        m_project.setName(projectName);
                });

                propertyRow("Project file", [&] {
                    char projectPath[512];
                    std::strncpy(projectPath, m_projectFilePath.string().c_str(), sizeof(projectPath));
                    projectPath[511] = '\0';
                    if (ImGui::InputText("##project_file", projectPath, sizeof(projectPath)))
                        m_projectFilePath = projectPath;
                });

                propertyRow("Script library", [&] {
                    char scriptLibPath[512];
                    std::strncpy(scriptLibPath, m_project.scriptLib().string().c_str(), sizeof(scriptLibPath));
                    scriptLibPath[511] = '\0';
                    if (ImGui::InputText("##script_lib", scriptLibPath, sizeof(scriptLibPath)))
                        m_project.setScriptLib(scriptLibPath);
                });

                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::SeparatorText("Game inputs");

            auto& inputContext = m_project.inputContext();

            if (ImGui::Button("Add input")) {
                std::size_t index = inputContext.inputs().size();
                std::string name;
                do name = std::format("input_{}", index++);
                while (inputContext.inputs().contains(name));
                inputContext.addInput(name, GE::ActionInput{});
            }

            std::string inputToRemove;
            std::pair<std::string, std::string> renameRequest;

            const float inputsHeight = std::max(180.0f, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() - 12.0f);
            ImGui::BeginChild("##project_inputs", ImVec2(0.0f, inputsHeight), true);
            for (auto& [inputName, vInput] : inputContext.inputs())
            {
                ImGui::PushID(inputName.c_str());

                const bool isOpen = collapsingHeaderWithActionButton(inputName.c_str(), "Remove", inputName, [&] {
                    inputToRemove = inputName;
                });
                if (isOpen)
                {
                    if (beginPropertyTable("##input_fields"))
                    {
                        char nameBuffer[256];
                        std::strncpy(nameBuffer, inputName.c_str(), sizeof(nameBuffer));
                        nameBuffer[255] = '\0';
                        propertyRow("Name", [&] {
                            if (ImGui::InputText("##name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                                renameRequest = { inputName, nameBuffer };
                        });

                        const char* currentKindLabel = std::visit([](const auto& input) -> const char*
                        {
                            using InputType = std::remove_cvref_t<decltype(input)>;
                            return EditableInputTraits<InputType>::label;
                        },
                        vInput);

                        propertyRow("Kind", [&] {
                            if (ImGui::BeginCombo("##kind", currentKindLabel))
                            {
                                GE::forEachType<GE::InputTypes>([&]<typename InputType>()
                                {
                                    const bool isSelected = std::holds_alternative<InputType>(vInput);
                                    if (ImGui::Selectable(EditableInputTraits<InputType>::label, isSelected))
                                        vInput = InputType{};
                                    if (isSelected)
                                        ImGui::SetItemDefaultFocus();
                                });
                                ImGui::EndCombo();
                            }
                        });

                        propertyRow("Mapper", [&] {
                            std::visit([](auto& input) {
                                const char* currentMapperLabel = NO_INPUT_MAPPER_LABEL;
                                if (!input.mapper)
                                    currentMapperLabel = NO_INPUT_MAPPER_LABEL;
                                std::visit([&](const auto& mapper)
                                {
                                    using MapperType = std::remove_cvref_t<decltype(mapper)>;
                                    using RawInputType = typename MapperType::RawInputType;
                                    currentMapperLabel = EditableRawInputTraits<RawInputType>::label;
                                },
                                *input.mapper);

                                if (ImGui::BeginCombo("##mapper", currentMapperLabel))
                                {
                                    const bool noMapperSelected = !input.mapper.has_value();

                                    if (ImGui::Selectable(NO_INPUT_MAPPER_LABEL, !input.mapper.has_value()))
                                        input.mapper.reset();
                                    if (!input.mapper.has_value())
                                        ImGui::SetItemDefaultFocus();

                                    GE::forEachType<GE::RawInputTypes>([&]<typename RawInputType>()
                                    {
                                        if constexpr (std::same_as<RawInputType, GE::KeyboardButton>)
                                        {
                                            using InputType = std::remove_cvref_t<decltype(input)>;
                                            using MapperType = typename InputType::template MapperType<RawInputType>;
                                            const bool isSelected = input.mapper && std::holds_alternative<MapperType>(*input.mapper);
                                            if (ImGui::Selectable(EditableRawInputTraits<RawInputType>::label, isSelected) && !isSelected)
                                                input.setMapper(MapperType{});
                                            if (isSelected)
                                                ImGui::SetItemDefaultFocus();
                                        }
                                    });

                                    ImGui::EndCombo();
                                }
                            },
                            vInput);
                        });

                        std::visit([&](auto& input)
                        {
                            using InputType = std::remove_cvref_t<decltype(input)>;
                            if (!input.mapper)
                                return;
                            auto& mapper = std::get<GE::InputMapper<GE::KeyboardButton, InputType>>(*input.mapper);
                            if constexpr (std::same_as<InputType, GE::Range2DInput>) {
                                propertyRow("X+ key",  [&] { keyboardButtonCombo("##x_pos", mapper.xPos); });
                                propertyRow("X- key",  [&] { keyboardButtonCombo("##x_neg", mapper.xNeg); });
                                propertyRow("X scale", [&] { ImGui::InputFloat("##x_scale", &mapper.xScale); });

                                propertyRow("Y+ key",  [&] { keyboardButtonCombo("##y_pos", mapper.yPos); });
                                propertyRow("Y- key",  [&] { keyboardButtonCombo("##y_neg", mapper.yNeg); });
                                propertyRow("Y scale", [&] { ImGui::InputFloat("##y_scale", &mapper.yScale); });

                                propertyRow("Trigger", [&] { ImGui::InputFloat("##trigger", &mapper.triggerValue); });
                            }
                            else if constexpr (std::same_as<InputType, GE::RangeInput>) {
                                propertyRow("Key",   [&] { keyboardButtonCombo("##key", mapper.button); });
                                propertyRow("Scale", [&] { ImGui::InputFloat("##scale", &mapper.scale); });
                            }
                            else {
                                propertyRow("Key", [&] { keyboardButtonCombo("##key", mapper.button); });
                            }
                        },
                        vInput);
                        ImGui::EndTable();
                    }
                    ImGui::Spacing();
                }

                ImGui::PopID();

                if (!inputToRemove.empty())
                    break;
            }
            ImGui::EndChild();

            if (!renameRequest.first.empty())
                inputContext.renameInput(renameRequest.first, renameRequest.second);
            if (!inputToRemove.empty())
                inputContext.removeInput(inputToRemove);

            ImGui::Spacing();

            if (ImGui::Button("Close", ImVec2(90.0f, 0.0f)))
                projectPropertiesOpen = false;
        }
        ImGui::End();
    }

    ImGui::Render();
}

}
