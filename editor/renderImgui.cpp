/*
 * ---------------------------------------------------
 * renderImgui.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Editor.hpp"
#include "EditorTheme.hpp"
#include "EditorUiState.hpp"
#include "imgui_render/icons.hpp"
#include "ImGuizmo.h"
#include "imgui_render/imgui_render.hpp"

#include <Game-Engine/AssetContainer.hpp>
#include <Game-Engine/AssetLocation.hpp>
#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/Components.hpp>
#include <Game-Engine/ECSRegistry.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/Scene.hpp>

#include <fastgltf/math.hpp>
#include <fastgltf/types.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <gfx_imgui/gfx_imgui.hpp>

#include <tracy/Tracy.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <format>
#include <functional>
#include <optional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <variant>

constexpr float TILE_SIZE = GE_Editor::Theme::resourceTileSize;
constexpr float VIEW_MANIPULATE_SIZE = 64.0f;
constexpr float VIEW_MANIPULATE_PADDING = 4.0f;
constexpr float VIEW_MANIPULATE_ORBIT_DISTANCE = 8.0f;

namespace
{

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

    auto& ui = *m_uiState;
    ImGuiIO& io = ImGui::GetIO();
    const auto [width, height] = window().size();
    const auto [framebufferWidth, framebufferHeight] = window().frameBufferSize();
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DisplayFramebufferScale = ImVec2(
        width > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(width) : 1.0f,
        height > 0 ? static_cast<float>(framebufferHeight) / static_cast<float>(height) : 1.0f
    );

    const auto currentTime = std::chrono::steady_clock::now();
    const float elapsedTime = ui.previousFrameTime == std::chrono::steady_clock::time_point{}
        ? 1.0f / 60.0f
        : std::chrono::duration<float>(currentTime - ui.previousFrameTime).count();
    io.DeltaTime = std::max(elapsedTime, 0.00001f);
    ui.previousFrameTime = currentTime;

    if (io.WantSetMousePos)
        window().setCursorPos(io.MousePos.x, io.MousePos.y);

    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
    {
        const ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
        const bool cursorVisible = imguiCursor != ImGuiMouseCursor_None && io.MouseDrawCursor == false;
        window().setCursorVisibility(cursorVisible);
        if (cursorVisible)
        {
            GE::Window::CursorShape cursorShape = GE::Window::CursorShape::arrow;
            switch (imguiCursor)
            {
                case ImGuiMouseCursor_Arrow:      cursorShape = GE::Window::CursorShape::arrow; break;
                case ImGuiMouseCursor_TextInput:  cursorShape = GE::Window::CursorShape::textInput; break;
                case ImGuiMouseCursor_ResizeAll:  cursorShape = GE::Window::CursorShape::resizeAll; break;
                case ImGuiMouseCursor_ResizeNS:   cursorShape = GE::Window::CursorShape::resizeNS; break;
                case ImGuiMouseCursor_ResizeEW:   cursorShape = GE::Window::CursorShape::resizeEW; break;
                case ImGuiMouseCursor_ResizeNESW: cursorShape = GE::Window::CursorShape::resizeNESW; break;
                case ImGuiMouseCursor_ResizeNWSE: cursorShape = GE::Window::CursorShape::resizeNWSE; break;
                case ImGuiMouseCursor_Hand:       cursorShape = GE::Window::CursorShape::hand; break;
                case ImGuiMouseCursor_NotAllowed: cursorShape = GE::Window::CursorShape::notAllowed; break;
                case ImGuiMouseCursor_None:
                case ImGuiMouseCursor_COUNT:
                default:
                    std::unreachable();
            }
            window().setCursorShape(cursorShape);
        }
    }

    TracyCZoneN(imguiNewFrameCtx, "Application::imgui new frame", true);
    gfx::imgui::newFrame(device());
    TracyCZoneEnd(imguiNewFrameCtx);

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    ImGui::DockSpaceOverViewport();

    const bool canSave = m_projectFilePath.has_value() && !m_projectFilePath->empty();
    const auto save = [this, &ui] {
        try {
            saveProject();
            ui.statusMessage = "Project saved";
            ui.statusUntil = ImGui::GetTime() + 3.0;
        }
        catch (const std::exception& error) {
            ui.statusMessage = std::format("Save failed: {}", error.what());
            ui.statusUntil = ImGui::GetTime() + 8.0;
        }
    };
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File")) {
            ImGui::MenuItem("New", nullptr, false, false);
            ImGui::MenuItem("Open", nullptr, false, false);
            if (ImGui::MenuItem("Save", nullptr, false, canSave)) save();
            ImGui::EndMenu();
        }
        menuFromPath(std::to_array<MenuItem>({
            {"Project/Properties",        [&ui]{ ui.projectPropertiesOpen = true; }},
            {"Project/Reload script library", m_scriptLibPath.has_value() && std::filesystem::exists(*m_scriptLibPath) && !m_game.has_value() ? [this]() { reloadScriptLib(); } : std::function<void()>()},
            {"Project/Run",               m_game.has_value() ? std::function<void()>() : [this]() { startGame(); }},
            {"Project/Stop",              m_game.has_value() ? [this]() { stopGame(); } : std::function<void()>()},
            {"Debug/Asset manager",       [&ui]{ ui.assetManagerWindowOpen = true; }},
        }));
        if (ImGui::GetTime() < ui.statusUntil) {
            ImGui::Separator();
            ImGui::TextUnformatted(ui.statusMessage.c_str());
        }
        ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("viewport"))
    {
        ZoneScopedN("ImGuiWindow(\"viewport\")");

        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImVec2 contentRegionAvai = ImGui::GetContentRegionAvail();

        ImGuizmo::SetRect(cursorPos.x, cursorPos.y, contentRegionAvai.x, contentRegionAvai.y);

        uint32_t newWidth = contentRegionAvai.x <= 0 ? 1 : static_cast<uint32_t>(contentRegionAvai.x);
        uint32_t newHeight = contentRegionAvai.y <= 0 ? 1 : static_cast<uint32_t>(contentRegionAvai.y);

        ImGui::Image(&ui.viewportTexture, contentRegionAvai);
        const bool viewportImageClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

        std::optional<std::pair<uint32_t, uint32_t>> viewportMousePosition;
        ImVec2 mousePos = ImGui::GetMousePos();
        if (mousePos.x >= cursorPos.x && mousePos.x < cursorPos.x + contentRegionAvai.x
            && mousePos.y >= cursorPos.y && mousePos.y < cursorPos.y + contentRegionAvai.y)
        {
            const float localX = mousePos.x - cursorPos.x;
            const float localY = mousePos.y - cursorPos.y;
            const uint32_t pixelX = std::min(static_cast<uint32_t>(localX / contentRegionAvai.x * static_cast<float>(newWidth)), newWidth - 1);
            const uint32_t pixelY = std::min(static_cast<uint32_t>(localY / contentRegionAvai.y * static_cast<float>(newHeight)), newHeight - 1);
            viewportMousePosition = {pixelX, pixelY};
        }

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

        bool transformGizmoRendered = false;
        bool toolbarHovered = false;
        if (m_game.has_value() == false) {
            const bool popupOpen = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
            const bool dragging = ImGuizmo::IsUsingAny() || ImGuizmo::IsUsingViewManipulate();
            const float toolbarInset = Theme::viewportToolbarInset;
            const bool toolbarBesideCube = contentRegionAvai.x >= viewportToolbarSize().x + VIEW_MANIPULATE_SIZE
                + VIEW_MANIPULATE_PADDING + toolbarInset*2;
            const ImVec2 toolbarTopRight(
                cursorPos.x+contentRegionAvai.x-toolbarInset-(toolbarBesideCube ? VIEW_MANIPULATE_SIZE+VIEW_MANIPULATE_PADDING : 0),
                cursorPos.y+toolbarInset+(toolbarBesideCube ? 0 : VIEW_MANIPULATE_SIZE+VIEW_MANIPULATE_PADDING));
            toolbarHovered = renderViewportToolbar(ui.gizmoSettings, toolbarTopRight, dragging);
            // The floating toolbar already owns its mouse input; hovering it need not grey the axes.
            ImGuizmo::Enable(dragging || !popupOpen);
            float aspectRatio = static_cast<float>(newWidth) / static_cast<float>(newHeight);
            glm::mat4x4 viewMatrix = m_editorCamera.viewMatrix();
            glm::mat4x4 projectionMatrix = m_editorCamera.projectionMatrix(aspectRatio);
            ImGuizmo::SetDrawlist();

            if (m_selectedEntity.has_value() && m_selectedEntity->has<GE::TransformComponent>()) {
                GE::TransformComponent& transformComponent = m_selectedEntity->get<GE::TransformComponent>();
                constexpr std::array<ImGuizmo::OPERATION, 3> operations = {ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE};
                const auto operation = operations.at(static_cast<int>(ui.gizmoSettings.tool));
                const auto mode = ui.gizmoSettings.space == GizmoSpace::world ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
                if (ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix), operation, mode, glm::value_ptr(transformComponent.worldTransform))) {
                    glm::mat4x4 localTransform = transformComponent.worldTransform;
                    auto parent = m_selectedEntity->parent();
                    if (parent.has_value() && parent->has<GE::TransformComponent>())
                        localTransform = glm::inverse(parent->worldTransform()) * transformComponent.worldTransform;
                    [[maybe_unused]] glm::vec3 skew;
                    [[maybe_unused]] glm::vec4 perspective;
                    glm::decompose(localTransform, transformComponent.scale, transformComponent.rotation, transformComponent.position, skew, perspective);
                    m_selectedEntity->updateTransformHierarchy();
                }
                transformGizmoRendered = true;
            }

            const ImVec2 viewManipulatePosition(
                cursorPos.x + std::max(0.0f, contentRegionAvai.x - VIEW_MANIPULATE_SIZE - VIEW_MANIPULATE_PADDING),
                cursorPos.y + VIEW_MANIPULATE_PADDING
            );
            const ImVec2 viewManipulateSize(VIEW_MANIPULATE_SIZE, VIEW_MANIPULATE_SIZE);
            ImGuizmo::ViewManipulate(
                glm::value_ptr(viewMatrix),
                VIEW_MANIPULATE_ORBIT_DISTANCE,
                viewManipulatePosition,
                viewManipulateSize,
                ImGui::ColorConvertFloat4ToU32(Theme::orientationBackground)
            );
            if (ImGuizmo::IsUsingViewManipulate()) {
                m_editorCamera.setViewMatrix(viewMatrix);
            }
        }

        if (viewportMousePosition && viewportImageClicked && m_game.has_value() == false && !toolbarHovered
            && (transformGizmoRendered == false || ImGuizmo::IsOver() == false)
            && ImGuizmo::IsUsingAny() == false
            && ImGuizmo::IsUsingViewManipulate() == false
            && ImGuizmo::IsViewManipulateHovered() == false)
        {
            m_viewportReadbackRequest = viewportMousePosition;
        }
    }
    ImGui::End();

    if (ImGui::Begin("Scene graph"))
    {
        assert(m_editedScene.has_value());
        renderSceneGraph(*m_editedScene, m_selectedEntity, assetManager(), ui.sceneGraph);
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
        ImGui::BeginDisabled(ui.resourceBrowserSubDir.empty());
        if (ImGui::Button("<"))
            ui.resourceBrowserSubDir = ui.resourceBrowserSubDir.parent_path();
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Parent folder");
        ImGui::SameLine();
        const std::string resourcePath = (m_resourceDir->filename() / ui.resourceBrowserSubDir).string();
        ImGui::TextUnformatted(resourcePath.c_str());
        ImGui::BeginChild("ResourcesChild");
        std::vector<std::filesystem::directory_entry> entries;
        for (const auto& entry : std::filesystem::directory_iterator(*m_resourceDir / ui.resourceBrowserSubDir))
            entries.push_back(entry);
        std::ranges::sort(entries, [](const auto& a, const auto& b) {
            if (a.is_directory() != b.is_directory()) return a.is_directory();
            return a.path().filename() < b.path().filename();
        });
        tileGrid(entries, std::function([&](const std::filesystem::directory_entry& entry){
            const std::string entryPath = entry.path().string();
            const std::string entryName = entry.path().filename().string();
            std::string extension = entry.path().extension().string();
            std::ranges::transform(extension, extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            const bool directory = entry.is_directory();
            const bool model = extension == ".glb" || extension == ".gltf" || extension == ".obj";
            const bool image = extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr" || extension == ".webp";
            const char* type = directory ? "Folder" : model ? "Model" : image ? "Image" : "File";
            ImGui::PushID(entryPath.c_str());
            ImGui::BeginGroup();
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, Theme::resourceTileRounding);
            ImGui::Button("##thumbnail", ImVec2(TILE_SIZE, TILE_SIZE));
            ImGui::PopStyleVar();
            // Vector placeholders share the square preview area with future thumbnails.
            const auto tileMin = ImGui::GetItemRectMin();
            (directory ? Icons::folder : Icons::file)(ImVec2(tileMin.x + TILE_SIZE * 0.5f, tileMin.y + TILE_SIZE * 0.5f),
                Theme::resourceIconSize, ImGui::GetColorU32(directory ? Theme::folder : Theme::file), Theme::iconStroke);
            if (directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                ui.resourceBrowserSubDir /= entry.path().filename();
            if (!directory && ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("resource_dnd", entryPath.c_str(), entryPath.size() + 1);
                ImGui::TextUnformatted(entryName.c_str());
                ImGui::EndDragDropSource();
            }
            const auto textMin = ImGui::GetCursorScreenPos();
            const ImVec2 textSize(TILE_SIZE, ImGui::GetTextLineHeightWithSpacing() * Theme::resourceLabelLines);
            const ImVec4 clip(textMin.x, textMin.y, textMin.x + textSize.x, textMin.y + textSize.y);
            ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), textMin,
                ImGui::GetColorU32(ImGuiCol_Text), entryName.c_str(), nullptr, TILE_SIZE, &clip);
            ImGui::Dummy(textSize);
            ImGui::EndGroup();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("%s\n%s%s", entryName.c_str(), type, directory ? " - double-click to open" : " - drag into the editor");
            if (ImGui::BeginPopupContextItem("resource_actions")) {
                const auto copyPath = [&](const std::filesystem::path& path) {
                    ImGui::SetClipboardText(path.string().c_str());
                    ui.statusMessage = "Path copied";
                    ui.statusUntil = ImGui::GetTime() + 3.0;
                };
                if (ImGui::MenuItem("Copy full path"))
                    copyPath(std::filesystem::absolute(entry.path()));
                if (ImGui::MenuItem("Copy resource-relative path"))
                    copyPath(entry.path().lexically_relative(*m_resourceDir));
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }));
        ImGui::EndChild();
    }
    ImGui::End();

    if (ui.assetManagerWindowOpen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x * 0.65f, viewport->Size.y * 0.45f), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Asset manager", &ui.assetManagerWindowOpen))
            renderAssetManagerDebugWindow(assetManager());
        ImGui::End();
    }

    if (ImGui::Begin("Entity inspector"))
    {
        ZoneScopedN("EntityInspectorWindow");
        ImGui::PushItemWidth(-110);
        if (!m_selectedEntity)
            ImGui::TextUnformatted("No entity selected");
        else
        {
            renderComponentInspector(*m_selectedEntity, assetManager(), m_scriptLibrary.has_value() ? &m_scriptLibrary.value() : nullptr, ui.componentInspector);
        }
        ImGui::PopItemWidth();
    }
    ImGui::End();

    if (ui.projectPropertiesOpen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 windowSize(viewport->Size.x * 0.42f, viewport->Size.y * 0.70f);
        ImGui::SetNextWindowPos(
            ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.5f),
            ImGuiCond_Appearing,
            ImVec2(0.5f, 0.5f)
        );
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);

        if (ImGui::Begin("Project properties", &ui.projectPropertiesOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
        {
            EditableProjectProperties properties{
                .projectName = m_projectName,
                .projectPath = m_projectFilePath,
                .scriptLibPath = m_scriptLibPath,
                .inputs = m_gameInputs
            };
            ui.projectPropertiesOpen = renderPropertiesWindow(properties);
        }
        ImGui::End();
    }

    ImGui::Render();
}

}
