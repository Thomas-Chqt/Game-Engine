#include "imgui_render.hpp"

#include <Game-Engine/RawInput.hpp>

#include <tracy/Tracy.hpp>

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <optional>
#include <string>
#include <utility>

namespace GE_Editor
{

namespace
{

constexpr const char* NO_INPUT_MAPPER_LABEL = "None";

template<typename T> struct InputTraits;
template<> struct InputTraits<GE::ActionInput>  { static constexpr const char* label = "Action"; };
template<> struct InputTraits<GE::StateInput>   { static constexpr const char* label = "State";  };
template<> struct InputTraits<GE::RangeInput>   { static constexpr const char* label = "Range";  };
template<> struct InputTraits<GE::Range2DInput> { static constexpr const char* label = "Range2D"; };

template<typename T> struct RawInputTraits;
template<> struct RawInputTraits<GE::KeyboardButton> { static constexpr const char* label = "Keyboard button"; };

bool beginPropertyTable(const char* id)
{
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV))
        return false;

    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 130.0f);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

template<typename Fn>
void propertyRow(const char* label, const Fn& fn)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    fn();
}

void keyboardButtonCombo(const char* label, GE::KeyboardButton& button)
{
    if (ImGui::BeginCombo(label, GE::keyboardButtonName(button).data())) // NOLINT(bugprone-suspicious-stringview-data-usage)
    {
        for (GE::KeyboardButton candidate : GE::KEYBOARD_BUTTONS)
        {
            const bool isSelected = (candidate == button);
            if (ImGui::Selectable(GE::keyboardButtonName(candidate).data(), isSelected)) // NOLINT(bugprone-suspicious-stringview-data-usage)
                button = candidate;
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

} // namespace

bool renderPropertiesWindow(EditableProjectProperties& properties)
{
    ZoneScoped;
    ImGui::Spacing();

    if (beginPropertyTable("##project_settings"))
    {
        propertyRow("Project name", [&] {
            strBuff<256>(properties.projectName, [&](char* buff) {
                if (ImGui::InputText("##project_name", buff, 256))
                    properties.projectName = std::string(buff);
            });
        });

        propertyRow("Project file", [&] {
            strBuff<512>(properties.projectPath ? properties.projectPath->c_str() : "", [&](char* buff) {
                if (ImGui::InputText("##project_file", buff, 512)) {
                    if (std::strlen(buff) == 0)
                        properties.projectPath = std::nullopt;
                    else
                        properties.projectPath = std::filesystem::path(buff);
                }
            });
        });

        propertyRow("Script library", [&] {
            strBuff<512>(properties.scriptLibPath ? properties.scriptLibPath->c_str() : "", [&](char* buff) {
                if (ImGui::InputText("##script_lib", buff, 512)) {
                    if (std::strlen(buff) == 0)
                        properties.scriptLibPath = std::nullopt;
                    else
                        properties.scriptLibPath = std::filesystem::path(buff);
                }
            });
        });

        ImGui::EndTable();
    }

    ImGui::SeparatorText("Game inputs");

    if (ImGui::Button("Add input"))
    {
        std::size_t index = properties.inputs.size();
        std::string name;
        do name = std::format("input_{}", index++);
        while (std::ranges::find(properties.inputs, name, &std::pair<std::string, GE::VInput>::first) != properties.inputs.end());
        properties.inputs.emplace_back(name, GE::ActionInput{});
    }

    const float inputsHeight = std::max(180.0f, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() - 12.0f);
    ImGui::BeginChild("##project_inputs", ImVec2(0.0f, inputsHeight), true);
    for (auto& [inputName, vInput] : properties.inputs)
    {
        ImGui::PushID(inputName.c_str());

        bool erased = false;
        const bool isOpen = collapsingHeaderWithActionButton(inputName.c_str(), "Remove", inputName, [&] {
            auto it = std::ranges::find(properties.inputs, inputName, &std::pair<std::string, GE::VInput>::first);
            properties.inputs.erase(it);
            erased = true;
        });
        if (erased)
            break;

        if (isOpen)
        {
            if (beginPropertyTable("##input_fields"))
            {
                propertyRow("Name", [&] {
                    strBuff<256>(inputName, [&](char* buff) {
                        if (ImGui::InputText("##script_lib", buff, 256, ImGuiInputTextFlags_EnterReturnsTrue))
                            inputName = std::string(buff);
                    });
                });

                const char* currentKindLabel = std::visit([]<typename T>(const T&) { return InputTraits<T>::label; }, vInput);

                propertyRow("Kind", [&] {
                    if (ImGui::BeginCombo("##kind", currentKindLabel))
                    {
                        GE::forEachType<GE::InputTypes>([&]<typename InputType> {
                            const bool isSelected = std::holds_alternative<InputType>(vInput);
                            if (ImGui::Selectable(InputTraits<InputType>::label, isSelected))
                                vInput = InputType{};
                            if (isSelected)
                                ImGui::SetItemDefaultFocus();
                        });
                        ImGui::EndCombo();
                    }
                });

                propertyRow("Mapper", [&] {
                    std::visit([]<typename InputType>(InputType& input) {
                        const char* currentMapperLabel = NO_INPUT_MAPPER_LABEL;
                        if (input.mapper)
                            std::visit([&]<typename T>(const T&) { currentMapperLabel = RawInputTraits<typename T::RawInputType>::label; }, *input.mapper);

                        if (ImGui::BeginCombo("##mapper", currentMapperLabel))
                        {
                            const bool noMapperSelected = !input.mapper.has_value();
                            if (ImGui::Selectable(NO_INPUT_MAPPER_LABEL, noMapperSelected))
                                input.mapper.reset();
                            if (noMapperSelected)
                                ImGui::SetItemDefaultFocus();

                            GE::forEachType<GE::RawInputTypes>([&]<typename RawInputType> {
                                if constexpr (std::same_as<RawInputType, GE::KeyboardButton>) { // lot of things not yet implemented for mouse buttons
                                    using MapperType = typename InputType::template MapperType<RawInputType>;
                                    const bool isSelected = input.mapper && std::holds_alternative<MapperType>(*input.mapper);
                                    if (ImGui::Selectable(RawInputTraits<RawInputType>::label, isSelected) && !isSelected)
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

                std::visit([]<typename InputType>(InputType& input)
                {
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
    }
    ImGui::EndChild();

    ImGui::Spacing();

    if (ImGui::Button("Close", ImVec2(90.0f, 0.0f)))
        return false;
    return true;
}

} // namespace GE_Editor
