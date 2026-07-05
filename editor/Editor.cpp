/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"
#include "Project.hpp"
#include "yaml_convert/convert_project.hpp"

#include <Game-Engine/BuiltInPasses.hpp>
#include <Game-Engine/Event.hpp>
#include <Game-Engine/RawInput.hpp>
#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/ECSView.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/FrameGraph.hpp>
#include <Game-Engine/Components.hpp>
#include <Game-Engine/Script.hpp>
#include <Game-Engine/Scene.hpp>
#include <Game-Engine/InputFwd.hpp>
#include <Game-Engine/InputContext.hpp>
#include <Game-Engine/ECSWorld.hpp>
#include <Game-Engine/Input.hpp>

#include <Graphics/Enums.hpp>

#include <yaml-cpp/yaml.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <format>
#include <fstream>
#include <memory>
#include <functional>
#include <print>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <cassert>
#include <cstddef>
#include <algorithm>
#include <cstdio>
#include <expected>
#include <imgui.h>
#include <ranges>
#include <string_view>
#include <tuple>
#include <filesystem>
#include <memory>
#include <utility>
#include <cassert>
#include <span>
#include <vector>

extern std::unique_ptr<GE::Application> createApplication(int argc, const char* argv[]) // NOLINT(cppcoreguidelines-avoid-c-arrays)
{
    return std::make_unique<GE_Editor::Editor>(std::span<const char*>{argv, static_cast<size_t>(argc)});
}

namespace GE_Editor
{

namespace
{

std::expected<Project, std::string> loadProjectFile(const std::filesystem::path& path)
{
    ZoneScoped;

    Project project;

    std::ifstream file(path);
    if (file.is_open() == false)
        return std::unexpected(std::format("unable to open file : {}", path.string()));

    const YAML::Node projectNode = YAML::Load(file);
    if (YAML::convert<Project>::decode(projectNode, project) == false)
        return std::unexpected(std::format("unable to load project file : {}", path.string()));

    return project;
}

}

Editor::Editor(std::span<const char*> args)
{
    loadProject(makeDefaultProject());

    if (args.size() == 2) {
        std::filesystem::path path = args[1];
        auto project = loadProjectFile(path);
        if (project.has_value()) {
            m_projectFilePath = path;
            loadProject(std::move(project.value()));
        }
        else {
            std::println(stderr, "{}", project.error());
        }
    }

    GE::Range2DInput editorCameraMoveInput;
    editorCameraMoveInput.setMapper<GE::KeyboardButton>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
        .xPos = GE::KeyboardButton::d, .xNeg = GE::KeyboardButton::a, .yPos = GE::KeyboardButton::w, .yNeg = GE::KeyboardButton::s,
    });
    m_editorInputContext.addInput("camera_move", editorCameraMoveInput);

    GE::Range2DInput editorCameraRotationInput;
    editorCameraRotationInput.setMapper<GE::KeyboardButton>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
        .xPos = GE::KeyboardButton::up, .xNeg = GE::KeyboardButton::down, .yPos = GE::KeyboardButton::right, .yNeg = GE::KeyboardButton::left,
    });
    m_editorInputContext.addInput("camera_rotate", editorCameraRotationInput);

    m_editorInputContext.setInputCallback<GE::Range2DInput>("camera_move", [this](const glm::vec2& value) { m_editorCamera.onMoveInput(value); }) ;
    m_editorInputContext.setInputCallback<GE::Range2DInput>("camera_rotate", [this](const glm::vec2& value) { m_editorCamera.onRotationInput(value); });

    pushInputContext(&m_editorInputContext);
    pushInputContext(&m_imguiInputContext);
}

void Editor::onUpdate()
{
    ZoneScopedN("Editor::onUpdate");
    processDropedFiles();

    if (m_game.has_value())
    {
        ZoneScopedN("Editor::game update");
        // game update
        // TODO ? maybe move into game class
        for (auto [scriptComponent] : m_game->activeScene().ecsWorld() | GE::ECSView<GE::ScriptComponent>())
        {
            if (scriptComponent.instance)
                scriptComponent.instance->onUpdate();
        }
    }

    renderImgui();
}

void Editor::onEvent(GE::Event& event)
{
    if (event.dispatch<GE::WindowRequestCloseEvent>([&](auto&) { terminate(); })) return;
}

void Editor::loadProject(Project&& project)
{
    ZoneScoped;

    if (m_game.has_value())
        stopGame();
    m_editedScene.reset();

    m_projectName = std::move(project.name);

    m_resourceDir = std::move(project.resourceDir);
    m_scriptLibPath = std::move(project.scriptLibPath);

    for (auto& [name, location, id, dependentAssets] : project.registeredAssets)
        assetManager().registerAsset(id, name, location, dependentAssets);
    m_gameInputs = project.inputs;

    m_sceneDescriptors = std::move(project.scenes);
    for(auto& scene : m_sceneDescriptors)
    {
        for (GE::Entity rootEntity : scene.ecsWorld | GE::ECSView<GE::NameComponent>() | GE::MakeEntity{} | std::views::filter([](GE::Entity entity){ return entity.hasParent() == false; }))
            rootEntity.updateTransformHierarchy();
    }
    m_startSceneName = std::move(project.startSceneName);

    m_editorCamera = project.editorCamera;
    editScene(project.editedSceneName);
    m_selectedEntity = project.selectedEntityId.transform([&](GE::EntityID id){ return GE::Entity{&m_editedScene->ecsWorld(), id}; });


    ImGui::LoadIniSettingsFromMemory(project.imguiSettings.c_str());

    if (m_scriptLibPath.has_value() && std::filesystem::exists(*m_scriptLibPath))
        reloadScriptLib();
    else
        m_scriptLibrary.reset();
}

void Editor::saveProject()
{
    assert(m_projectFilePath.has_value() && m_projectFilePath->empty() == false);

    if (m_editedScene)
        syncEditedScene();

    std::map<GE::AssetID, std::tuple<std::string, GE::VAssetLocation, std::vector<GE::AssetID>>> registeredAssets;

    auto addRegisteredAsset = [&](this auto&& self, GE::AssetID id) -> void {
        for (GE::AssetID childId : assetManager().assetDependencies(id))
            self(childId);
            if (assetManager().assetLocation(id).has_value()) {
                registeredAssets.try_emplace(id,
                    std::string(assetManager().assetName(id)),
                    assetManager().assetLocation(id).value(),
                    assetManager().assetDependencies(id) | std::ranges::to<std::vector>()
                );
            }
    };

    auto topAssetIds = m_sceneDescriptors
        | std::views::transform([](const GE::Scene::Descriptor& desc) { return desc.ecsWorld | GE::const_ECSView<GE::MeshComponent>(); })
        | std::views::join
        | std::views::transform([](const auto& e){ return static_cast<GE::AssetID>(e.template get<0>()); })
        | std::ranges::to<std::set>()
        | std::views::filter([&](GE::AssetID id) -> bool { return assetManager().assetLocation(id).has_value(); });

    for (GE::AssetID id : topAssetIds)
        addRegisteredAsset(id);

    Project project{
        .name = m_projectName,

        .resourceDir = m_resourceDir,
        .scriptLibPath = m_scriptLibPath,

        .registeredAssets = registeredAssets | std::views::transform([](const auto& pair) -> std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>> {
            return std::make_tuple(
                std::get<0>(pair.second),
                std::get<1>(pair.second),
                pair.first,
                std::get<2>(pair.second)
            );
        }) | std::ranges::to<std::vector>(),

        .inputs = m_gameInputs,

        .scenes = m_sceneDescriptors,
        .startSceneName = m_startSceneName,

        .editorCamera = m_editorCamera,
        .editedSceneName = m_editedScene.transform([](const GE::Scene& scene){ return scene.name(); }),
        .selectedEntityId = m_selectedEntity.transform([](const GE::Entity& entity){ return entity.entityId; }),

        .imguiSettings = ImGui::SaveIniSettingsToMemory(),
    };


    if (m_projectFilePath->has_parent_path())
        std::filesystem::create_directories(m_projectFilePath->parent_path());

    std::ofstream file(*m_projectFilePath);
    if (!file.is_open())
        throw std::runtime_error("failed to open project file for writing");

    YAML::Emitter out;
    out << YAML::convert<Project>::encode(project);
    if (!out.good())
        throw std::runtime_error("failed to serialize project");

    file << out.c_str();
    if (!file.good())
        throw std::runtime_error("failed to write project file");
}

void Editor::editScene(std::optional<std::string_view> name)
{
    if (m_editedScene)
        syncEditedScene();

    m_editedScene.reset();

    if (name.has_value()) {
        auto it = std::ranges::find(m_sceneDescriptors, *name, &GE::Scene::Descriptor::name);
        assert(it != m_sceneDescriptors.end());
        m_editedScene.reset();
        m_editedScene.emplace(&assetManager(), *it);
    }
}

void Editor::reloadScriptLib()
{
    assert(m_scriptLibPath);
    assert(std::filesystem::exists(*m_scriptLibPath));
    assert(m_game.has_value() == false);
    assert(m_editedScene.has_value() == false || std::ranges::all_of(
        m_editedScene->ecsWorld() | GE::ECSView<GE::ScriptComponent>(),
        [](const auto& e) -> bool { return e.template get<0>().instance == nullptr; }
    ));
    m_scriptLibrary.emplace(*m_scriptLibPath);
}

void Editor::startGame()
{
    assert(m_game.has_value() == false);
    syncEditedScene();
    m_game.emplace(GE::Game::Descriptor{
        .assetManager = &assetManager(),
        .inputContext = GE::InputContext(m_gameInputs),
        .scenes = m_sceneDescriptors,
        .startSceneName = m_startSceneName,
        .scriptLibrary = m_scriptLibrary ? &m_scriptLibrary.value() : nullptr,
    });
    setPrimaryInputContext(m_game->inputContext());
}

void Editor::stopGame()
{
    assert(m_game.has_value());
    setPrimaryInputContext(m_editorInputContext);
    m_game.reset();
}

void Editor::setPrimaryInputContext(GE::InputContext& inputContext)
{
    popInputContext();
    popInputContext();

    pushInputContext(&inputContext);
    pushInputContext(&m_imguiInputContext);
}

void Editor::processDropedFiles()
{
    ZoneScopedN("Editor::processDropedFiles");

    while (const std::optional<std::filesystem::path> droppedFile = window().popDroppedFile())
    {
        if (std::filesystem::is_regular_file(*droppedFile) && droppedFile->extension() == ".geproj") {
            auto project = loadProjectFile(*droppedFile);
            if (project.has_value()) {
                loadProject(std::move(project.value()));
            }
            else {
                std::println(stderr, "{}", project.error());
            }
        }
    }
}

void Editor::recordFrameGraph(GE::FrameGraphBuilder& builder)
{
    ZoneScopedN("Editor::recordFrameGraph");

    const GE::Scene& scene = [&]() -> const GE::Scene& {
        if (m_game.has_value())
            return m_game->activeScene();
        assert(m_editedScene);
        return *m_editedScene;
    }();

    GE::FrameGraph::TextureRef viewportBackBuffer = builder.newTexture("backBuffer", m_viewportSize, gfx::PixelFormat::BGRA8Unorm);
    builder.aliasTexture("viewportBackBuffer", viewportBackBuffer);
    builder.newTexture("depthBuffer", m_viewportSize, gfx::PixelFormat::Depth32Float);
    auto windowBackBuffer = builder.newTexture("windowBackBuffer", window().frameBufferSize(), gfx::PixelFormat::BGRA8Unorm);
    builder.setBackBuffer(windowBackBuffer);

    assert(m_viewportSize.second != 0);
    const float aspectRatio = static_cast<float>(m_viewportSize.first) / static_cast<float>(m_viewportSize.second);

    glm::mat4 viewProjectionMatrix;
    glm::vec3 cameraPosition;
    if (m_game.has_value())
    {
        GE::const_Entity activeCamera = scene.activeCamera();
        assert(activeCamera.world != nullptr);
        assert(activeCamera.world->isValidEntityID(activeCamera.entityId));
        assert(activeCamera.has<GE::TransformComponent>());
        assert(activeCamera.has<GE::CameraComponent>());

        glm::vec3 position;
        glm::quat rotation;

        [[maybe_unused]] glm::vec3 scale;
        [[maybe_unused]] glm::vec3 skew;
        [[maybe_unused]] glm::vec4 perspective;

        glm::decompose(activeCamera.worldTransform(), scale, rotation, position, skew, perspective);

        const glm::vec3 direction = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        const glm::vec3 up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);

        viewProjectionMatrix = activeCamera.get<GE::CameraComponent>().projectionMatrix(aspectRatio) * glm::lookAt(position, position + direction, up);
        cameraPosition = position;
    }
    else
    {
        viewProjectionMatrix = m_editorCamera.viewProjectionMatrix(aspectRatio);
        cameraPosition = m_editorCamera.position();
    }

    GE::TexturedGeometryPass{scene, viewProjectionMatrix, cameraPosition}
        .record(builder);

    GE::ImGuiPass{}
        .record(builder);
}

void Editor::syncEditedScene()
{
    assert(m_editedScene);
    auto it = std::ranges::find(m_sceneDescriptors, m_editedScene->name(), &GE::Scene::Descriptor::name);
    assert(it != m_sceneDescriptors.end());
    *it = GE::Scene::Descriptor{
        .name = m_editedScene->name(),
        .ecsWorld = m_editedScene->ecsWorld(),
        .activeCameraId = m_editedScene->activeCamera().entityId
    };
}

} // namespace GE_Editor
