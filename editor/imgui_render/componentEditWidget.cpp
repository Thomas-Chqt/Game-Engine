#include "imgui_render.hpp"

#include <imgui.h>
#include <numbers>
#include <string>

namespace GE_Editor
{

template<>
void componentEditWidget<GE::NameComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*)
{
    GE::NameComponent& nameComponent = entity.get<GE::NameComponent>();
    strBuff<32>(nameComponent.name, [&](char* cstr){
        if (ImGui::InputText("Name##NameComponent_name", cstr, 32))
            nameComponent.name = std::string(cstr);
    });
}

template<>
void componentEditWidget<GE::TransformComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*)
{
    GE::TransformComponent& transform = entity.get<GE::TransformComponent>();

    floatBuff<3>(transform.position, [&](float* buff){
        if (ImGui::DragFloat3("position##TransformComponent_position", buff, 0.01f, -1000.0f, 1000.0f))
            transform.position = glm::vec3{buff[0], buff[1], buff[2]};
    });
    floatBuff<3>(transform.rotation, [&](float* buff){
        if (ImGui::DragFloat3("rotation##TransformComponent_rotation", buff, 0.01f, -2*std::numbers::pi_v<float>, 2*std::numbers::pi_v<float>))
            transform.rotation = glm::vec3{buff[0], buff[1], buff[2]};
    });
    floatBuff<3>(transform.scale, [&](float* buff){
        if (ImGui::DragFloat3("scale##TransformComponent_scale", buff, 0.01f, 0.0f, 10.0f))
            transform.scale = glm::vec3{buff[0], buff[1], buff[2]};
    });
}

template<>
void componentEditWidget<GE::CameraComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*)
{
    GE::CameraComponent& cameraComponent = entity.get<GE::CameraComponent>();

    ImGui::DragFloat("fov##CameraComponent_fov", &cameraComponent.fov, 0.01f, -2*std::numbers::pi_v<float>, 2*std::numbers::pi_v<float>);
    ImGui::DragFloat("zFar##CameraComponent_zFar", &cameraComponent.zFar, 0.01f, 0.001f, 10000.0f);
    ImGui::DragFloat("zNear##CameraComponent_zNear", &cameraComponent.zNear, 0.01f, 0.001f, 10000.0f);
}

template<>
void componentEditWidget<GE::LightComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*)
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
    floatBuff<3>(lightComponent.color, [&](float* buff){
        if (ImGui::ColorEdit3("color##LightComponent_color", buff))
            lightComponent.color = glm::vec3{buff[0], buff[1], buff[2]};
    });
    ImGui::DragFloat("intentsity##LightComponent_intensity", &lightComponent.intentsity, 0.01, 0.0f, 1.0f);
    if (lightComponent.type == GE::LightComponent::Type::point)
        ImGui::DragFloat("attenuation##LightComponent_attenuation", &lightComponent.attenuation, 0.01, 0.0f, 1.0f);
}

template<>
void componentEditWidget<GE::ScriptComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary* scriptLibrary)
{
    if (scriptLibrary == nullptr) {
        ImGui::TextDisabled("No script library loaded"); // NOLINT(cppcoreguidelines-pro-type-vararg)
        return;
    }

    GE::ScriptComponent& scriptComponent = entity.get<GE::ScriptComponent>();
    const char* previewValue = scriptComponent.name.empty() ? "none" : scriptComponent.name.c_str();
    if (ImGui::BeginCombo("Script##ScriptComponent_name", previewValue))
    {
        for (const std::string& scriptName : scriptLibrary->listScriptNames())
        {
            const bool isSelected = scriptComponent.name == scriptName;
            if (ImGui::Selectable(scriptName.c_str(), isSelected))
            {
                scriptComponent.name = scriptName;
                scriptComponent.parameters.clear();
                for (const std::string& parameterName : scriptLibrary->listScriptParameterNames(scriptName))
                {
                    auto [parameterIt, inserted] = scriptComponent.parameters.try_emplace(parameterName, scriptLibrary->getScriptDefaultParameterValue(scriptName, parameterName));
                    assert(inserted);
                }
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    for (auto& [name, vScriptValue] : scriptComponent.parameters)
    {
        std::visit([&]<typename T>(T& value)
        {
            if constexpr (std::is_same_v<T, bool>) {
                ImGui::Checkbox(name.c_str(), &value);
            }
            else if constexpr (std::is_same_v<T, int64_t>) {
                int64_t value64 = value;
                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &value64))
                    value = value64;
            }
            else if constexpr (std::is_same_v<T, float>) {
                float fValue = value;
                if (ImGui::DragFloat(name.c_str(), &fValue, 0.01f))
                    value = fValue;
            }
            else if constexpr (std::is_same_v<T, glm::vec2>) {
                floatBuff<2>(value, [&](float* buff){
                    if (ImGui::DragFloat2(name.c_str(), buff, 0.01f))
                        value = glm::vec2{buff[0], buff[1]};
                });
            }
            else if constexpr (std::is_same_v<T, glm::vec3>) {
                floatBuff<3>(value, [&](float* buff){
                    if (ImGui::DragFloat3(name.c_str(), buff, 0.01f))
                        value = glm::vec3{buff[0], buff[1], buff[2]};
                });
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                strBuff<256>(value, [&](char* buff){
                    if (ImGui::InputText(name.c_str(), buff, 256))
                        value = std::string(buff);
                });
            }
        }, vScriptValue);
    }
}

template<>
void componentEditWidget<GE::MeshComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager& assetManager, [[maybe_unused]] const GE::ScriptLibrary*)
{
    GE::MeshComponent& mesh = entity.get<GE::MeshComponent>();

    if (ImGui::BeginCombo("Mesh##RegistredMesh", assetManager.assetName(mesh).data())) // NOLINT(bugprone-suspicious-stringview-data-usage)
    {
        for (const GE::AssetID& assetId : assetManager.assetIds() | std::views::filter([&](const GE::AssetID& id) { return assetManager.assetTypeIs<GE::Mesh>(id); }))
        {
            const bool is_selected = (assetId == mesh.id);
            if (ImGui::Selectable(assetManager.assetName(assetId).data(), is_selected)) { // NOLINT(bugprone-suspicious-stringview-data-usage)
                assetManager.unloadAsset(mesh.id);
                mesh.id = assetId;
                assetManager.loadAsset(mesh.id);
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_dnd"); payload && payload->IsDelivery())
        {
            assert(payload->DataSize > 0);
            std::filesystem::path path = std::string_view(static_cast<const char*>(payload->Data), static_cast<std::size_t>(payload->DataSize - 1));
            GE::AssetID id = assetManager.registerAsset<GE::Mesh>(path.stem().string(), path);
            if (mesh.id != id)
            {
                assetManager.unloadAsset(mesh.id);
                mesh.id = id;
                assetManager.loadAsset(mesh.id);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

}
