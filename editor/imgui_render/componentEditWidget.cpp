#include "Game-Engine/Components.hpp"
#include "imgui_render.hpp"
#include "EditorTheme.hpp"

#include <glm/gtc/quaternion.hpp>

#include <imgui.h>

#include <string>

namespace GE_Editor
{

namespace
{

bool transformVector(const char* label, glm::vec3& value, float speed, float minimum, float maximum)
{
    ImGui::PushID(label);
    const float available = ImGui::GetContentRegionAvail().x;
    const float labelWidth = Theme::componentLabelWidth;
    const float gap = ImGui::GetStyle().ItemSpacing.x;
    const bool inlineLabel = available >= labelWidth + gap*3 + Theme::componentAxisMinWidth*3;
    const float labelX = ImGui::GetCursorPosX();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && std::string_view(label).starts_with("Rotation"))
        ImGui::SetTooltip("Rotation in degrees");
    if (inlineLabel) ImGui::SameLine(labelX+labelWidth+gap);
    const float width = (ImGui::GetContentRegionAvail().x - gap*2) / 3.0f;
    const std::array<const char*, 3> formats = {"X %.3f", "Y %.3f", "Z %.3f"};
    bool changed = false;
    for (int axis = 0; axis < 3; ++axis) {
        if (axis) ImGui::SameLine();
        ImGui::PushID(axis);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, Theme::axes[axis]);
        // Axis names stay visible even at zero; the colored underline helps scanning.
        ImGui::SetNextItemWidth(width);
        changed |= ImGui::DragFloat("##value", &value[axis], speed, minimum, maximum, formats[axis]);
        const auto min = ImGui::GetItemRectMin();
        const auto max = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddLine(ImVec2(min.x, max.y), max, ImGui::ColorConvertFloat4ToU32(Theme::axes[axis]), Theme::axisLineWidth);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
            ImGui::SetTooltip("%s axis - drag to adjust", axis == 0 ? "X" : axis == 1 ? "Y" : "Z");
        ImGui::PopStyleColor();
        ImGui::PopID();
    }
    ImGui::PopID();
    return changed;
}

} // namespace

template<>
void componentEditWidget<GE::TransformComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*)
{
    auto& transform = entity.get<GE::TransformComponent>();
    bool changed = transformVector("Position", transform.position, 0.01f, -1000.0f, 1000.0f);
    glm::vec3 degrees = glm::degrees(glm::eulerAngles(transform.rotation));
    if (transformVector("Rotation°", degrees, 0.1f, -360.0f, 360.0f)) {
        transform.rotation = glm::normalize(glm::quat(glm::radians(degrees)));
        changed = true;
    }
    changed |= transformVector("Scale", transform.scale, 0.01f, 0.0f, 10.0f);
    if (changed) entity.updateTransformHierarchy();
}

template<>
void componentEditWidget<GE::CameraComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*)
{
    auto& camera = entity.get<GE::CameraComponent>();
    float fov = glm::degrees(camera.fov);
    if (ImGui::DragFloat("Field of view", &fov, 0.1f, 1.0f, 179.0f, "%.1f deg", ImGuiSliderFlags_AlwaysClamp))
        camera.fov = glm::radians(fov);
    ImGui::DragFloat("Near clip", &camera.zNear, 0.01f, 0.001f, camera.zFar - 0.001f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Far clip", &camera.zFar, 0.1f, camera.zNear + 0.001f, 10000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
}

template<>
void componentEditWidget<GE::LightComponent>(GE::Entity entity, [[maybe_unused]] GE::AssetManager&, [[maybe_unused]] const GE::ScriptLibrary*)
{
    auto typeToStr = [](const GE::LightComponent::Type& type){
        switch (type) {
            case GE::LightComponent::Type::directional: return "Directional";
            case GE::LightComponent::Type::point: return "Point";
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
        if (ImGui::ColorEdit3("Color##LightComponent_color", buff))
            lightComponent.color = glm::vec3{buff[0], buff[1], buff[2]};
    });
    ImGui::DragFloat("Intensity##LightComponent_intensity", &lightComponent.intentsity, 0.01, 0.0f, 1.0f);
    if (lightComponent.type == GE::LightComponent::Type::point)
        ImGui::DragFloat("Attenuation##LightComponent_attenuation", &lightComponent.attenuation, 0.01, 0.0f, 1.0f);
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
                    scriptComponent.parameters.emplace_back(parameterName, scriptLibrary->getScriptDefaultParameterValue(scriptName, parameterName));
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
