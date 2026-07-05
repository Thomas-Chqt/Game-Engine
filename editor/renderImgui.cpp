/*
 * ---------------------------------------------------
 * renderImgui.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Editor.hpp"
#include "ImGuizmo.h"
#include "imgui_render/imgui_render.hpp"

#include <Game-Engine/AssetContainer.hpp>
#include <Game-Engine/AssetLocation.hpp>
#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/Components.hpp>
#include <Game-Engine/ECSWorld.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/Scene.hpp>

#include <fastgltf/math.hpp>
#include <fastgltf/types.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

#include <tracy/Tracy.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cctype>
#include <filesystem>
#include <format>
#include <functional>
#include <optional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <variant>

constexpr float TILE_SIZE = 60.0f;
constexpr float VIEW_MANIPULATE_SIZE = 64.0f;
constexpr float VIEW_MANIPULATE_PADDING = 4.0f;
constexpr float VIEW_MANIPULATE_ORBIT_DISTANCE = 8.0f;

namespace
{

using EditableEntityComponents = GE::TypeList<GE::NameComponent, GE::TransformComponent, GE::CameraComponent, GE::LightComponent, GE::ScriptComponent, GE::MeshComponent>;

template<typename T> struct EditableEntityComponentTraits;
template<> struct EditableEntityComponentTraits<GE::NameComponent>      { static constexpr const char* label = "Name component";      };
template<> struct EditableEntityComponentTraits<GE::TransformComponent> { static constexpr const char* label = "Transform component"; };
template<> struct EditableEntityComponentTraits<GE::CameraComponent>    { static constexpr const char* label = "Camera component";    };
template<> struct EditableEntityComponentTraits<GE::LightComponent>     { static constexpr const char* label = "Light component";     };
template<> struct EditableEntityComponentTraits<GE::ScriptComponent>    { static constexpr const char* label = "Script component";    };
template<> struct EditableEntityComponentTraits<GE::MeshComponent>      { static constexpr const char* label = "Mesh component";      };

bool isGltfPath(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::ranges::transform(extension, extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".gltf" || extension == ".glb";
}

std::string gltfNodeName(const fastgltf::Asset& asset, const std::filesystem::path& path, std::size_t nodeIndex)
{
    const fastgltf::Node& node = asset.nodes.at(nodeIndex);
    if (node.name.empty() == false)
        return std::string(node.name);
    if (node.meshIndex.has_value() && asset.meshes.at(*node.meshIndex).name.empty() == false)
        return std::string(asset.meshes.at(*node.meshIndex).name);
    return std::format("{}#node{}", path.stem().string(), nodeIndex);
}

std::tuple<glm::vec3, glm::quat, glm::vec3> gltfNodeLocalTransform(const fastgltf::Node& node)
{
    if (const fastgltf::TRS* nodeTRS = std::get_if<fastgltf::TRS>(&node.transform)) {
        const glm::vec3 position = glm::make_vec3(nodeTRS->translation.data());
        const glm::quat rotation(nodeTRS->rotation.w(), nodeTRS->rotation.x(), nodeTRS->rotation.y(), nodeTRS->rotation.z());
        const glm::vec3 scale = glm::make_vec3(nodeTRS->scale.data());
        return {position, rotation, scale};
    }

    if (const fastgltf::math::fmat4x4* nodeTransform = std::get_if<fastgltf::math::fmat4x4>(&node.transform)) {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
        [[maybe_unused]] glm::vec3 skew;
        [[maybe_unused]] glm::vec4 perspective;
        glm::decompose(glm::make_mat4x4(nodeTransform->data()), scale, rotation, position, skew, perspective);
        return {position, rotation, scale};
    }

    std::unreachable();
}

void importDroppedGltf(const std::filesystem::path& path, GE::AssetManager& assetManager, GE::Scene& scene)
{
    assert(isGltfPath(path));

    std::shared_ptr<GE::AssetContainer> container = assetManager.assetContainer(path);

    assetManager.importGltf(path);

    auto* gltfContainer = dynamic_cast<GE::GltfAssetContainer*>(container.get());
    assert(gltfContainer);

    const fastgltf::Asset& asset = gltfContainer->asset();
    assert(asset.scenes.empty() == false);
    const fastgltf::Scene& gltfScene = asset.defaultScene ? asset.scenes[*asset.defaultScene] : asset.scenes.front();

    auto createEntity = [&](this auto&& self, std::size_t nodeIndex, std::optional<GE::Entity> parent) -> GE::Entity
    {
        const fastgltf::Node& node = asset.nodes.at(nodeIndex);
        GE::Entity entity = scene.newEntity(gltfNodeName(asset, path, nodeIndex));
        const auto [position, rotation, scale] = gltfNodeLocalTransform(node);
        entity.emplace<GE::TransformComponent>(position, rotation, scale);

        if (parent.has_value())
            parent->addChild(entity);

        if (node.meshIndex.has_value()) {
            const GE::AssetLocation<GE::Mesh> meshLocation{
                .containerPath = path,
                .index = *node.meshIndex
            };
            const GE::AssetID meshAssetId = assetManager.assetId(GE::VAssetLocation(meshLocation));
            entity.emplace<GE::MeshComponent>(meshAssetId);
            assetManager.loadAsset(meshAssetId);
        }

        for (std::size_t childNodeIndex : node.children)
            self(childNodeIndex, entity);

        return entity;
    };

    for (std::size_t nodeIndex : gltfScene.nodeIndices)
        createEntity(nodeIndex, std::nullopt).updateTransformHierarchy();
}

} // namespace

namespace GE_Editor
{

void Editor::renderImgui()
{
    ZoneScopedN("Editor::renderImgui");

    static bool projectPropertiesOpen = false;
    static bool assetManagerWindowOpen = false;
    static std::filesystem::path resourceBrowserSubDir;

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    ImGui::DockSpaceOverViewport();

    if (ImGui::BeginMainMenuBar())
    {
        menuFromPath(std::to_array<MenuItem>({
            {"file/new",                  {}},
            {"file/open",                 {}},
            {"file/save",                 m_projectFilePath.has_value() && !m_projectFilePath->empty() ? [this] { saveProject(); } : std::function<void()>()},
            {"project/properties",        []{ projectPropertiesOpen = true; }},
            {"project/reload script lib", m_scriptLibPath.has_value() && std::filesystem::exists(*m_scriptLibPath) && !m_game.has_value() ? [this]() { reloadScriptLib(); } : std::function<void()>()},
            {"project/run",               m_game.has_value() ? std::function<void()>() : [this]() { startGame(); }},
            {"project/stop",              m_game.has_value() ? [this]() { stopGame(); } : std::function<void()>()},
            {"debug/asset manager",       []{ assetManagerWindowOpen = true; }},

        }));
    }
    ImGui::EndMainMenuBar();

    if (ImGui::Begin("viewport"))
    {
        ZoneScopedN("ImGuiWindow(\"viewport\")");
        static std::variant<std::string, uint64_t> textureIdPlaceholder = std::string("viewportBackBuffer");

        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImVec2 contentRegionAvai = ImGui::GetContentRegionAvail();

        ImGuizmo::SetRect(cursorPos.x, cursorPos.y, contentRegionAvai.x, contentRegionAvai.y);

        uint32_t newWidth = contentRegionAvai.x <= 0 ? 1 : static_cast<uint32_t>(contentRegionAvai.x);
        uint32_t newHeight = contentRegionAvai.y <= 0 ? 1 : static_cast<uint32_t>(contentRegionAvai.y);

        ImGui::Image(&textureIdPlaceholder, contentRegionAvai);

        if (newWidth != m_viewportSize.first || newHeight != m_viewportSize.second) {
            m_viewportSize = {newWidth, newHeight};
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_dnd");
                payload && payload->IsDelivery() && m_game.has_value() == false && m_editedScene.has_value())
            {
                assert(payload->DataSize > 0);
                std::filesystem::path path = std::string_view(static_cast<const char*>(payload->Data), static_cast<std::size_t>(payload->DataSize - 1));
                if (isGltfPath(path))
                    importDroppedGltf(path, assetManager(), *m_editedScene);
            }
            ImGui::EndDragDropTarget();
        }

        if (m_game.has_value() == false) {
            float aspectRatio = static_cast<float>(newWidth) / static_cast<float>(newHeight);
            glm::mat4x4 viewMatrix = m_editorCamera.viewMatrix();
            glm::mat4x4 projectionMatrix = m_editorCamera.projectionMatrix(aspectRatio);
            ImGuizmo::SetDrawlist();

            if (m_selectedEntity.has_value() && m_selectedEntity->has<GE::TransformComponent>()) {
                GE::TransformComponent& transformComponent = m_selectedEntity->get<GE::TransformComponent>();
                ImGuizmo::OPERATION operations = ImGuizmo::OPERATION::TRANSLATE | ImGuizmo::OPERATION::ROTATE;
                if (ImGuizmo::Manipulate((float*)&viewMatrix, (float*)&projectionMatrix, operations, ImGuizmo::MODE::LOCAL, (float*)&transformComponent.worldTransform)) {
                    glm::mat4x4 localTransform = transformComponent.worldTransform;
                    auto parent = m_selectedEntity->parent();
                    if (parent.has_value() && parent->has<GE::TransformComponent>())
                        localTransform = glm::inverse(parent->worldTransform()) * transformComponent.worldTransform;
                    [[maybe_unused]] glm::vec3 skew;
                    [[maybe_unused]] glm::vec4 perspective;
                    glm::decompose(localTransform, transformComponent.scale, transformComponent.rotation, transformComponent.position, skew, perspective);
                    m_selectedEntity->updateTransformHierarchy();
                }
            }

            glm::mat4x4 identityMatrix(1.0f);
            const ImVec2 viewManipulatePosition(
                cursorPos.x + std::max(0.0f, contentRegionAvai.x - VIEW_MANIPULATE_SIZE - VIEW_MANIPULATE_PADDING),
                cursorPos.y + VIEW_MANIPULATE_PADDING
            );
            const ImVec2 viewManipulateSize(VIEW_MANIPULATE_SIZE, VIEW_MANIPULATE_SIZE);
            ImGuizmo::ViewManipulate(
                (float*)&viewMatrix,
                (float*)&projectionMatrix,
                ImGuizmo::OPERATION::ROTATE,
                ImGuizmo::MODE::LOCAL,
                (float*)&identityMatrix,
                VIEW_MANIPULATE_ORBIT_DISTANCE,
                viewManipulatePosition,
                viewManipulateSize,
                IM_COL32(16, 16, 16, 96)
            );
            if (ImGuizmo::IsUsingViewManipulate()) {
                m_editorCamera.setViewMatrix(viewMatrix);
            }
        }
    }
    ImGui::End();

    if (ImGui::Begin("Scene graph"))
    {
        assert(m_editedScene.has_value());
        renderSceneGraph(*m_editedScene, m_selectedEntity, assetManager());
    }
    ImGui::End();

    if (ImGui::Begin("Scenes"))
    {
        ZoneScopedN("ImGuiWindow(\"Scenes\")");
        tileGrid(m_sceneDescriptors, std::function([](const GE::Scene::Descriptor& scene){
            ImGui::BeginGroup();
            {
                ImGui::Button(scene.name.c_str(), ImVec2(TILE_SIZE, TILE_SIZE));
                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("scene_dnd", scene.name.c_str(), scene.name.size());
                    ImGui::TextUnformatted(scene.name.c_str());
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

    if (ImGui::Begin("Resources") && m_resourceDir.has_value() && std::filesystem::exists(*m_resourceDir))
    {
        ZoneScopedN("ImGuiWindow(\"Resources\")");
        ImGui::BeginDisabled(resourceBrowserSubDir.empty());
        if (ImGui::Button("<"))
            resourceBrowserSubDir = resourceBrowserSubDir.parent_path();
        ImGui::EndDisabled();
        ImGui::SameLine();
        const std::string resourcePath = (m_resourceDir->filename() / resourceBrowserSubDir).string();
        ImGui::TextUnformatted(resourcePath.c_str());
        ImGui::BeginChild("ResourcesChild");
        tileGrid(std::filesystem::directory_iterator(*m_resourceDir / resourceBrowserSubDir), std::function([](const std::filesystem::directory_entry& entry){
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
                        ImGui::TextUnformatted(entryName.c_str());
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

    if (assetManagerWindowOpen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x * 0.65f, viewport->Size.y * 0.45f), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Asset manager", &assetManagerWindowOpen))
            renderAssetManagerDebugWindow(assetManager());
        ImGui::End();
    }

    if (ImGui::Begin("Entity inspector"))
    {
        ZoneScopedN("EntityInspectorWindow");
        ImGui::PushItemWidth(-80);
        if (!m_selectedEntity || m_selectedEntity->world == nullptr || m_selectedEntity->entityId == GE::INVALID_ENTITY_ID)
            ImGui::TextUnformatted("No entity selected");
        else
        {
            GE::forEachType<EditableEntityComponents>([&]<typename ComponentT>
            {
                if (m_selectedEntity->has<ComponentT>() && collapsingHeaderWithActionButton(EditableEntityComponentTraits<ComponentT>::label, "Remove", EditableEntityComponentTraits<ComponentT>::label, [&]{
                    if constexpr (std::same_as<ComponentT, GE::MeshComponent>) {
                        assert(assetManager().isValidAssetId(m_selectedEntity->get<GE::MeshComponent>().id));
                        assetManager().unloadAsset(m_selectedEntity->get<GE::MeshComponent>().id);
                    }
                    m_selectedEntity->remove<ComponentT>();
                }))
                {
                    componentEditWidget<ComponentT>(*m_selectedEntity, assetManager(), m_scriptLibrary.has_value() ? &m_scriptLibrary.value() : nullptr);
                }
            });

            ImGui::Separator();

            if (ImGui::Button("Add component", ImVec2(-FLT_MIN, 0.0f)))
                ImGui::OpenPopup("add_component_popup");

            if (ImGui::BeginPopup("add_component_popup"))
            {
                GE::forEachType<EditableEntityComponents>([&]<typename ComponentT>() {
                    if (m_selectedEntity->has<ComponentT>() == false && ImGui::Selectable(EditableEntityComponentTraits<ComponentT>::label))
                    {
                        m_selectedEntity->emplace<ComponentT>();
                        if constexpr (std::same_as<ComponentT, GE::MeshComponent>)
                            assetManager().loadAsset(m_selectedEntity->get<GE::MeshComponent>().id);
                    }
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
            EditableProjectProperties properties{
                .projectName = m_projectName,
                .projectPath = m_projectFilePath,
                .scriptLibPath = m_scriptLibPath,
                .inputs = m_gameInputs
            };
            projectPropertiesOpen = renderPropertiesWindow(properties);
        }
        ImGui::End();
    }

    ImGui::Render();
}

}
