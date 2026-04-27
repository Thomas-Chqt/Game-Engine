/*
 * ---------------------------------------------------
 * ProjectPropertiesPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "UI/ProjectPropertiesPanel.hpp"

#include <imgui.h>

#include <algorithm>
#include <cfloat>
#include <cstring>
#include <format>
#include <string>

namespace
{

constexpr const char* NO_INPUT_MAPPER_LABEL = "None";

template<typename T> struct EditableInputTraits;
template<> struct EditableInputTraits<GE::ActionInput> { static constexpr const char* label = "Action"; };
template<> struct EditableInputTraits<GE::StateInput>  { static constexpr const char* label = "State";  };
template<> struct EditableInputTraits<GE::RangeInput>  { static constexpr const char* label = "Range";  };
template<> struct EditableInputTraits<GE::Range2DInput> { static constexpr const char* label = "Range2D"; };

template<typename T> struct EditableRawInputTraits;
template<> struct EditableRawInputTraits<GE::KeyboardButton> { static constexpr const char* label = "Keyboard button"; };

bool beginPropertyTable(const char* id)
{
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV))
        return false;

    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 130.0f);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

template<typename Fn>
void renderPropertyRow(const char* label, Fn&& fn)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    fn();
}

bool renderKeyboardButtonCombo(const char* label, GE::KeyboardButton& button)
{
    bool changed = false;

    if (ImGui::BeginCombo(label, GE::keyboardButtonName(button).data()))
    {
        for (GE::KeyboardButton candidate : GE::KEYBOARD_BUTTONS)
        {
            const bool isSelected = (candidate == button);
            if (ImGui::Selectable(GE::keyboardButtonName(candidate).data(), isSelected))
            {
                button = candidate;
                changed = true;
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    return changed;
}

template<GE::InputType InputType>
void renderMapperFields(InputType& input)
{
    if (!input.mapper)
        return;

    auto& mapper = std::get<GE::InputMapper<GE::KeyboardButton, InputType>>(*input.mapper);
    if constexpr (std::same_as<InputType, GE::Range2DInput>)
    {
        renderPropertyRow("X+ key", [&] { renderKeyboardButtonCombo("##x_pos", mapper.xPos); });
        renderPropertyRow("X- key", [&] { renderKeyboardButtonCombo("##x_neg", mapper.xNeg); });
        renderPropertyRow("X scale", [&] { ImGui::InputFloat("##x_scale", &mapper.xScale); });
        renderPropertyRow("Y+ key", [&] { renderKeyboardButtonCombo("##y_pos", mapper.yPos); });
        renderPropertyRow("Y- key", [&] { renderKeyboardButtonCombo("##y_neg", mapper.yNeg); });
        renderPropertyRow("Y scale", [&] { ImGui::InputFloat("##y_scale", &mapper.yScale); });
        renderPropertyRow("Trigger", [&] { ImGui::InputFloat("##trigger", &mapper.triggerValue); });
    }
    else if constexpr (std::same_as<InputType, GE::RangeInput>)
    {
        renderPropertyRow("Key", [&] { renderKeyboardButtonCombo("##key", mapper.button); });
        renderPropertyRow("Scale", [&] { ImGui::InputFloat("##scale", &mapper.scale); });
    }
    else
        renderPropertyRow("Key", [&] { renderKeyboardButtonCombo("##key", mapper.button); });
}

template<GE::InputType InputType>
const char* currentMapperLabel(const InputType& input)
{
    if (!input.mapper)
        return NO_INPUT_MAPPER_LABEL;

    const char* label = NO_INPUT_MAPPER_LABEL;
    std::visit([&](const auto& mapper)
    {
        using MapperType = std::remove_cvref_t<decltype(mapper)>;
        using RawInputType = typename MapperType::RawInputType;
        label = EditableRawInputTraits<RawInputType>::label;
    },
    *input.mapper);
    return label;
}

template<GE::InputType InputType>
void renderMapperSelector(InputType& input)
{
    if (!ImGui::BeginCombo("##mapper", currentMapperLabel(input)))
        return;

    const bool noMapperSelected = !input.mapper.has_value();
    if (ImGui::Selectable(NO_INPUT_MAPPER_LABEL, noMapperSelected))
        input.mapper.reset();
    if (noMapperSelected)
        ImGui::SetItemDefaultFocus();

    GE::forEachType<GE::RawInputTypes>([&]<typename RawInputType>()
    {
        if constexpr (std::same_as<RawInputType, GE::KeyboardButton>)
        {
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

std::string makeInputName(const GE::InputContext& inputContext)
{
    std::size_t index = inputContext.inputs().size();
    std::string name;

    do
    {
        name = std::format("input_{}", index++);
    }
    while (inputContext.inputs().contains(name));

    return name;
}

bool inputEditHeader(const std::string& inputName, bool& removeRequested)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const float buttonWidth = ImGui::CalcTextSize("Remove").x + style.FramePadding.x * 2.0f;
    const float buttonX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonWidth;

    const bool isOpen = ImGui::CollapsingHeader(
        inputName.c_str(),
        ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen
    );
    ImGui::SameLine(buttonX);
    removeRequested = ImGui::SmallButton("Remove");
    return isOpen;
}

} // namespace

namespace GE_Editor
{

ProjectPropertiesPanel::ProjectPropertiesPanel(Project* project, std::filesystem::path* projectFilePath, bool* isOpen)
    : m_project(project)
    , m_projectFilePath(projectFilePath)
    , m_isOpen(isOpen)
{
}

void ProjectPropertiesPanel::render()
{
    if (m_project == nullptr || m_projectFilePath == nullptr || m_isOpen == nullptr || *m_isOpen == false)
        return;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 windowSize(viewport->Size.x * 0.42f, viewport->Size.y * 0.70f);
    ImGui::SetNextWindowPos(
        ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.5f),
        ImGuiCond_Appearing,
        ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Project properties", m_isOpen, flags))
    {
        ImGui::Spacing();

        char projectName[256];
        std::strncpy(projectName, m_project->name().c_str(), sizeof(projectName));
        projectName[255] = '\0';

        char projectPath[512];
        std::strncpy(projectPath, m_projectFilePath->string().c_str(), sizeof(projectPath));
        projectPath[511] = '\0';

        char scriptLibPath[512];
        std::strncpy(scriptLibPath, m_project->scriptLib().string().c_str(), sizeof(scriptLibPath));
        scriptLibPath[511] = '\0';
        const auto& scenes = m_project->scenes();
        const auto currentStartScene = scenes.find(m_project->startScene().first);

        if (beginPropertyTable("##project_settings"))
        {
            renderPropertyRow("Project name", [&] {
                if (ImGui::InputText("##project_name", projectName, sizeof(projectName)))
                    m_project->setName(projectName);
            });

            renderPropertyRow("Project file", [&] {
                if (ImGui::InputText("##project_file", projectPath, sizeof(projectPath)))
                    *m_projectFilePath = projectPath;
            });

            renderPropertyRow("Script library", [&] {
                if (ImGui::InputText("##script_lib", scriptLibPath, sizeof(scriptLibPath)))
                    m_project->setScriptLib(scriptLibPath);
            });

            renderPropertyRow("Start scene", [&] {
                const char* previewValue = currentStartScene != scenes.end() ? currentStartScene->second.name.c_str() : "none";
                if (ImGui::BeginCombo("##start_scene", previewValue))
                {
                    for (const auto& [sceneId, descriptor] : scenes)
                    {
                        const bool isSelected = (sceneId == m_project->startScene().first);
                        if (ImGui::Selectable(descriptor.name.c_str(), isSelected))
                            m_project->setStartScene(sceneId);
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            });

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Game inputs");

        if (ImGui::Button("Add input"))
            m_project->inputContext().addInput(makeInputName(m_project->inputContext()), GE::ActionInput{});

        auto& inputContext = m_project->inputContext();

        std::string inputToRemove;
        std::pair<std::string, std::string> renameRequest;

        const float inputsHeight = std::max(180.0f, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() - 12.0f);
        ImGui::BeginChild("##project_inputs", ImVec2(0.0f, inputsHeight), true);
        for (auto& [inputName, input] : inputContext.inputs())
        {
            ImGui::PushID(inputName.c_str());

            bool removeRequested = false;
            const bool isOpen = inputEditHeader(inputName, removeRequested);
            if (removeRequested)
                inputToRemove = inputName;

            if (isOpen)
            {
                if (beginPropertyTable("##input_fields"))
                {
                    char nameBuffer[256];
                    std::strncpy(nameBuffer, inputName.c_str(), sizeof(nameBuffer));
                    nameBuffer[255] = '\0';
                    renderPropertyRow("Name", [&] {
                        if (ImGui::InputText("##name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                            renameRequest = { inputName, nameBuffer };
                    });

                    const char* currentKindLabel = std::visit([](const auto& value) -> const char*
                    {
                        using InputType = std::remove_cvref_t<decltype(value)>;
                        return EditableInputTraits<InputType>::label;
                    },
                    input);

                    renderPropertyRow("Kind", [&] {
                        if (ImGui::BeginCombo("##kind", currentKindLabel))
                        {
                            GE::forEachType<GE::InputTypes>([&]<typename InputType>()
                            {
                                const bool isSelected = std::holds_alternative<InputType>(input);
                                if (ImGui::Selectable(EditableInputTraits<InputType>::label, isSelected))
                                    input = InputType{};
                                if (isSelected)
                                    ImGui::SetItemDefaultFocus();
                            });
                            ImGui::EndCombo();
                        }
                    });

                    renderPropertyRow("Mapper", [&] { std::visit([](auto& value) { renderMapperSelector(value); }, input); });
                    std::visit([](auto& value) { renderMapperFields(value); }, input);
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
            *m_isOpen = false;
    }
    ImGui::End();
}

} // namespace GE_Editor
