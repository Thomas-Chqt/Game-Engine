/*
 * ---------------------------------------------------
 * renderImgui.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Editor.hpp"
#include "EditorTheme.hpp"
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
#include <concepts>
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

void drawResourceIcon(ImVec2 center, bool directory)
{
    namespace Theme = GE_Editor::Theme;
    auto* draw = ImGui::GetWindowDrawList();
    const float size = Theme::resourceIconSize;
    const auto point = [&](float x, float y) { return ImVec2(center.x + x * size, center.y + y * size); };
    const auto color = ImGui::ColorConvertFloat4ToU32(directory ? Theme::folder : Theme::file);
    if (directory) {
        const std::array<ImVec2, 6> outline = {point(-0.50f, -0.33f), point(-0.12f, -0.33f),
            point(0.0f, -0.20f), point(0.50f, -0.20f), point(0.50f, 0.36f), point(-0.50f, 0.36f)};
        draw->AddPolyline(outline.data(), outline.size(), color, ImDrawFlags_Closed, Theme::iconStroke);
        draw->AddLine(point(-0.50f, -0.10f), point(0.50f, -0.10f), color, Theme::iconStroke);
    }
    else {
        const std::array<ImVec2, 5> outline = {point(-0.32f, -0.46f), point(0.10f, -0.46f),
            point(0.34f, -0.22f), point(0.34f, 0.46f), point(-0.32f, 0.46f)};
        draw->AddPolyline(outline.data(), outline.size(), color, ImDrawFlags_Closed, Theme::iconStroke);
        const std::array<ImVec2, 3> fold = {point(0.10f, -0.46f), point(0.10f, -0.22f), point(0.34f, -0.22f)};
        draw->AddPolyline(fold.data(), fold.size(), color, 0, Theme::iconStroke);
        draw->AddLine(point(-0.18f, 0.07f), point(0.18f, 0.07f), color, Theme::iconStroke);
        draw->AddLine(point(-0.18f, 0.23f), point(0.10f, 0.23f), color, Theme::iconStroke);
    }
}

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

    ImGuiIO& io = ImGui::GetIO();
    const auto [width, height] = window().size();
    const auto [framebufferWidth, framebufferHeight] = window().frameBufferSize();
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DisplayFramebufferScale = ImVec2(
        width > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(width) : 1.0f,
        height > 0 ? static_cast<float>(framebufferHeight) / static_cast<float>(height) : 1.0f
    );

    static std::chrono::steady_clock::time_point previousFrameTime;
    const auto currentTime = std::chrono::steady_clock::now();
    const float elapsedTime = previousFrameTime == std::chrono::steady_clock::time_point{}
        ? 1.0f / 60.0f
        : std::chrono::duration<float>(currentTime - previousFrameTime).count();
    io.DeltaTime = std::max(elapsedTime, 0.00001f);
    previousFrameTime = currentTime;

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

    static bool projectPropertiesOpen = false;
    static bool assetManagerWindowOpen = false;
    static std::filesystem::path resourceBrowserSubDir;

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    ImGui::DockSpaceOverViewport();

    const bool canSave = m_projectFilePath.has_value() && !m_projectFilePath->empty();
    const auto save = [this] {
        try { saveProject(); }
        catch (const std::exception& error) {
            m_statusMessage = std::format("Save failed: {}", error.what());
            m_statusUntil = ImGui::GetTime() + 8.0;
        }
    };
    // Route globally, including while a text field has focus. Cmd on macOS, Ctrl elsewhere.
    if (canSave && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
        save();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File")) {
            ImGui::MenuItem("New", nullptr, false, false);
            ImGui::MenuItem("Open", nullptr, false, false);
#ifdef __APPLE__
            const char* saveShortcut = "Cmd+S";
#else
            const char* saveShortcut = "Ctrl+S";
#endif
            if (ImGui::MenuItem("Save", saveShortcut, false, canSave)) save();
            ImGui::EndMenu();
        }
        menuFromPath(std::to_array<MenuItem>({
            {"Project/Properties",        []{ projectPropertiesOpen = true; }},
            {"Project/Reload script library", m_scriptLibPath.has_value() && std::filesystem::exists(*m_scriptLibPath) && !m_game.has_value() ? [this]() { reloadScriptLib(); } : std::function<void()>()},
            {"Project/Run",               m_game.has_value() ? std::function<void()>() : [this]() { startGame(); }},
            {"Project/Stop",              m_game.has_value() ? [this]() { stopGame(); } : std::function<void()>()},
            {"Debug/Asset manager",       []{ assetManagerWindowOpen = true; }},
        }));
        if (ImGui::GetTime() < m_statusUntil) {
            ImGui::Separator();
            ImGui::TextUnformatted(m_statusMessage.c_str());
        }
        ImGui::EndMainMenuBar();
    }

    m_viewportCameraActive = false;
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
        const bool viewportImageClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
        const bool viewportHovered = ImGui::IsItemHovered();

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
            m_viewportCameraActive = viewportHovered && ImGui::IsMouseDown(ImGuiMouseButton_Right)
                && !io.WantTextInput && !popupOpen && !dragging;
            updateGizmoShortcuts(m_gizmoSettings, !dragging && !ImGui::IsMouseDown(ImGuiMouseButton_Right));
            const float toolbarInset = Theme::viewportToolbarInset;
            const bool toolbarBesideCube = contentRegionAvai.x >= viewportToolbarSize().x + VIEW_MANIPULATE_SIZE
                + VIEW_MANIPULATE_PADDING + toolbarInset*2;
            const ImVec2 toolbarTopRight(
                cursorPos.x+contentRegionAvai.x-toolbarInset-(toolbarBesideCube ? VIEW_MANIPULATE_SIZE+VIEW_MANIPULATE_PADDING : 0),
                cursorPos.y+toolbarInset+(toolbarBesideCube ? 0 : VIEW_MANIPULATE_SIZE+VIEW_MANIPULATE_PADDING));
            toolbarHovered = renderViewportToolbar(m_gizmoSettings, toolbarTopRight, dragging);
            // The floating toolbar already owns its mouse input; hovering it need not grey the axes.
            // Preserve the existing guards for camera navigation and open popups, and keep active drags alive.
            ImGuizmo::Enable(dragging || (!m_viewportCameraActive && !popupOpen));
            float aspectRatio = static_cast<float>(newWidth) / static_cast<float>(newHeight);
            glm::mat4x4 viewMatrix = m_editorCamera.viewMatrix();
            glm::mat4x4 projectionMatrix = m_editorCamera.projectionMatrix(aspectRatio);
            ImGuizmo::SetDrawlist();

            if (m_selectedEntity.has_value() && m_selectedEntity->has<GE::TransformComponent>()) {
                GE::TransformComponent& transformComponent = m_selectedEntity->get<GE::TransformComponent>();
                constexpr std::array<ImGuizmo::OPERATION, 3> operations = {ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE};
                const auto operation = operations[static_cast<int>(m_gizmoSettings.tool)];
                const auto mode = m_gizmoSettings.space == GizmoSpace::world ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
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
            && !ImGui::IsMouseDown(ImGuiMouseButton_Right)
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
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("Parent folder");
        ImGui::SameLine();
        const std::string resourcePath = (m_resourceDir->filename() / resourceBrowserSubDir).string();
        ImGui::TextUnformatted(resourcePath.c_str());
        ImGui::BeginChild("ResourcesChild");
        std::vector<std::filesystem::directory_entry> entries;
        for (const auto& entry : std::filesystem::directory_iterator(*m_resourceDir / resourceBrowserSubDir))
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
            drawResourceIcon(ImVec2(tileMin.x + TILE_SIZE * 0.5f, tileMin.y + TILE_SIZE * 0.5f), directory);
            if (directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                resourceBrowserSubDir /= entry.path().filename();
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
                    m_statusMessage = "Path copied";
                    m_statusUntil = ImGui::GetTime() + 3.0;
                };
                if (ImGui::MenuItem("Copy full path")) copyPath(std::filesystem::absolute(entry.path()));
                if (ImGui::MenuItem("Copy resource-relative path")) copyPath(entry.path().lexically_relative(*m_resourceDir));
                ImGui::EndPopup();
            }
            ImGui::PopID();
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
        ImGui::PushItemWidth(-110);
        if (!m_selectedEntity)
            ImGui::TextUnformatted("No entity selected");
        else
        {
            renderComponentInspector(*m_selectedEntity, assetManager(), m_scriptLibrary.has_value() ? &m_scriptLibrary.value() : nullptr);
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
