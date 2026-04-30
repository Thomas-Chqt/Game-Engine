/*
 * ---------------------------------------------------
 * EntityInspectorPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/04 15:10:00
 * ---------------------------------------------------
 */

#include "UI/EntityInspectorPanel.hpp"
#include "Game-Engine/AssetManagerView.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Game-Engine/Scene.hpp"

#include <Game-Engine/Components.hpp>
#include <Game-Engine/ECSWorld.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/ScriptLibraryManager.hpp>
#include <Game-Engine/Script.hpp>
#include <Game-Engine/TypeList.hpp>

#include <cstddef>
#include <filesystem>
#include <imgui.h>

#include <cstring>
#include <cassert>
#include <cstdint>
#include <numbers>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace GE_Editor
{

using EditableEntityComponents = GE::TypeList<
    GE::TransformComponent,
    GE::CameraComponent,
    GE::LightComponent,
    GE::ScriptComponent,
    GE::MeshComponent
>;

template<typename T>
struct EditableEntityComponentTraits;

template<> struct EditableEntityComponentTraits<GE::TransformComponent> { static constexpr const char* label = "Transform component"; };
template<> struct EditableEntityComponentTraits<GE::CameraComponent>    { static constexpr const char* label = "Camera component";    };
template<> struct EditableEntityComponentTraits<GE::LightComponent>     { static constexpr const char* label = "Light component";     };
template<> struct EditableEntityComponentTraits<GE::ScriptComponent>    { static constexpr const char* label = "Script component";    };
template<> struct EditableEntityComponentTraits<GE::MeshComponent>      { static constexpr const char* label = "Mesh component";      };

template<>
void EntityInspectorPanel::componentEditWidget<GE::NameComponent>()
{
    GE::NameComponent& nameComponent = m_entity.get<GE::NameComponent>();
    char buff[32];
    std::strncpy(buff, nameComponent.name.c_str(), sizeof(buff));
    buff[31] = '\0'; // If count is reached before the entire string src was copied, the resulting character array is not null-terminated.
    ImGui::InputText("Name##NameComponent_name", buff, sizeof(buff));
    nameComponent.name = std::string(buff);
}

template<>
void EntityInspectorPanel::componentEditWidget<GE::TransformComponent>()
{
    GE::TransformComponent& transform = m_entity.get<GE::TransformComponent>();
    ImGui::DragFloat3("position##TransformComponent_position", (float*)&transform.position, 0.01f, -1000.0f, 1000.0f);
    ImGui::DragFloat3("rotation##TransformComponent_rotation", (float*)&transform.rotation, 0.01f, -2*std::numbers::pi_v<float>, 2*std::numbers::pi_v<float>);
    ImGui::DragFloat3("scale##TransformComponent_scale",       (float*)&transform.scale,    0.01f, 0.0f, 10.0f);
}

template<>
void EntityInspectorPanel::componentEditWidget<GE::CameraComponent>()
{
    GE::CameraComponent& cameraComponent = m_entity.get<GE::CameraComponent>();

    ImGui::DragFloat("fov##CameraComponent_fov",     &cameraComponent.fov,   0.01f,  -2*std::numbers::pi_v<float>, 2*std::numbers::pi_v<float>);
    ImGui::DragFloat("zFar##CameraComponent_zFar",   &cameraComponent.zFar,  0.01f, 0.001f, 10000.0f);
    ImGui::DragFloat("zNear##CameraComponent_zNear", &cameraComponent.zNear, 0.01f, 0.001f, 10000.0f);
}

template<>
void EntityInspectorPanel::componentEditWidget<GE::LightComponent>()
{
    auto typeToStr = [](const GE::LightComponent::Type& type){
        switch (type) {
            case GE::LightComponent::Type::directional: return "directional";
            case GE::LightComponent::Type::point: return "point";
        }
        std::unreachable();
        return "";
    };

    GE::LightComponent& lightComponent = m_entity.get<GE::LightComponent>();
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

template<>
void EntityInspectorPanel::componentEditWidget<GE::ScriptComponent>()
{
    GE::ScriptComponent& scriptComponent = m_entity.get<GE::ScriptComponent>();
    if (!m_listScriptNames || !m_listScriptParameters)
    {
        ImGui::TextDisabled("No script library loaded");
        return;
    }

    const char* previewValue = scriptComponent.name.empty() ? "none" : scriptComponent.name.c_str();
    if (ImGui::BeginCombo("Script##ScriptComponent_name", previewValue))
    {
        for (const std::string& scriptName : m_listScriptNames())
        {
            const bool isSelected = scriptComponent.name == scriptName;
            if (ImGui::Selectable(scriptName.c_str(), isSelected))
            {
                scriptComponent.name = scriptName;
                scriptComponent.parameters.clear();
                for (const GE::ScriptParameterDescriptor& parameter : m_listScriptParameters(scriptName))
                {
                    auto [parameterIt, inserted] = scriptComponent.parameters.try_emplace(parameter.name, parameter.defaultValue);
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
void EntityInspectorPanel::componentEditWidget<GE::MeshComponent>()
{
    assert(m_scene);

    GE::MeshComponent& meshComponent = m_entity.get<GE::MeshComponent>();

    std::string currentMeshStem;
    if (meshComponent.id == GE::BUILT_IN_CUBE_ASSET_ID)
        currentMeshStem = "built_in_cube";
    else
    {
        const auto& registredAssets = m_scene->assetManagerView().registredAssets();
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

        for (auto& [vAssetPath, id] : m_scene->assetManagerView().registredAssets())
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
            GE::AssetID id = m_scene->assetManagerView().registerAsset<GE::Mesh>(std::filesystem::path(path));
            meshComponent.id = id;
        }
        ImGui::EndDragDropTarget();
    }
}

EntityInspectorPanel::EntityInspectorPanel(GE::Entity entity, GE::Scene* scene, GE::ListScriptNamesFn listScriptNames, GE::ListScriptParametersFn listScriptParameters)
    : m_entity(std::move(entity))
    , m_scene(scene)
    , m_listScriptNames(std::move(listScriptNames))
    , m_listScriptParameters(std::move(listScriptParameters))
{
}

void EntityInspectorPanel::render()
{
    if (ImGui::Begin("Entity inspector"))
    {
        ImGui::PushItemWidth(-80);
        if (m_entity.world == nullptr || m_entity.entityId == INVALID_ENTITY_ID)
            ImGui::Text("No entity selected");
        else
        {
            componentEditWidget<GE::NameComponent>();
            GE::forEachType<EditableEntityComponents>([&]<typename ComponentT>() {
                if (m_entity.has<ComponentT>() && componentEditHeader<ComponentT>(EditableEntityComponentTraits<ComponentT>::label))
                    componentEditWidget<ComponentT>();
            });

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Add component"))
                ImGui::OpenPopup("add_component_popup");
            addComponentPopUp();

            ImGui::SameLine();

            if (m_onEntityDelete && ImGui::Button("Delete enitity"))
                m_onEntityDelete();
        }
        ImGui::PopItemWidth();
    }
    ImGui::End();
}

template<typename T>
bool EntityInspectorPanel::componentEditHeader(const char* title)
{
    bool isOpen = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::SameLine(0, 30);
    if (ImGui::Button((std::string("remove##") + std::string(title)).c_str()))
        return m_entity.remove<T>(), false;
    return isOpen;
}

void EntityInspectorPanel::addComponentPopUp()
{
    if (ImGui::BeginPopup("add_component_popup"))
    {
        GE::forEachType<EditableEntityComponents>([&]<typename ComponentT>() {
            if (m_entity.has<ComponentT>() == false && ImGui::Selectable(EditableEntityComponentTraits<ComponentT>::label))
                m_entity.emplace<ComponentT>();
        });

        ImGui::EndPopup();
    }
}

}
