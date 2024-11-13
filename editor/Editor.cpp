/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"
#include "ECS/Components.hpp"
#include "ECS/ECSView.hpp"
#include "ECS/Entity.hpp"
#include "EditorCamera.hpp"
#include "Graphics/Event.hpp"
#include "InputManager/RawInput.hpp"
#include "InputManager/Mapper.hpp"
#include "Project.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene.hpp"
#include "Script.hpp"
#include "UI/ContentBrowserPanel.hpp"
#include "UI/EntityInspectorPanel.hpp"
#include "UI/FileOpenDialog.hpp"
#include "UI/FileSaveDialog.hpp"
#include "UI/MainMenuBar.hpp"
#include "UI/ProjectPropertiesModal.hpp"
#include "UI/ViewportPanel.hpp"
#include "UI/SceneGraphPanel.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "ViewportFrameBuff.hpp"
#include "imgui.h"
#include <stb_image/stb_image.h>
#include <dlLoad/dlLoad.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace GE
{

Editor::Editor() : m_vpFrameBuff(window(), renderer().graphicAPI())
{
    ImGui::GetIO().IniFilename = nullptr;

    Mapper<KeyboardButton, Range2DInput>::Descriptor inputMapperDesc;

    ActionInput& quitEditorIpt = m_editorInputContext.newInput<ActionInput>("quit_editor");
    quitEditorIpt.callback = utils::Func<void()>(*(Application*)this, &Application::terminate);
    auto quitEditorIptMapper = utils::makeUnique<Mapper<KeyboardButton, ActionInput>>(KeyboardButton::esc, quitEditorIpt);
    quitEditorIpt.mappers[0] = quitEditorIptMapper.staticCast<IMapper>();
    quitEditorIpt.mappers[1].clear();

    Range2DInput& editorCamMoveIpt = m_editorInputContext.newInput<Range2DInput>("editor_cam_move");
    editorCamMoveIpt.callback = utils::Func<void(math::vec2f)>(m_editorCamera, &EditorCamera::move);
    inputMapperDesc.xPos = KeyboardButton::d;
    inputMapperDesc.xNeg = KeyboardButton::a;
    inputMapperDesc.yPos = KeyboardButton::w;
    inputMapperDesc.yNeg = KeyboardButton::s;
    auto editorCamMoveIptMapper = utils::makeUnique<Mapper<KeyboardButton, Range2DInput>>(inputMapperDesc, editorCamMoveIpt);
    editorCamMoveIpt.mappers[0] = editorCamMoveIptMapper.staticCast<IMapper>();
    editorCamMoveIpt.mappers[1].clear();

    Range2DInput& editorCamRotateIpt = m_editorInputContext.newInput<Range2DInput>("editor_cam_rotate");
    editorCamRotateIpt.callback = utils::Func<void(math::vec2f)>(m_editorCamera, &EditorCamera::rotate);
    inputMapperDesc.xPos = KeyboardButton::down;
    inputMapperDesc.xNeg = KeyboardButton::up;
    inputMapperDesc.yPos = KeyboardButton::right;
    inputMapperDesc.yNeg = KeyboardButton::left;
    auto editorCamRotateIptMapper = utils::makeUnique<Mapper<KeyboardButton, Range2DInput>>(inputMapperDesc, editorCamRotateIpt);
    editorCamRotateIpt.mappers[0] = editorCamRotateIptMapper.staticCast<IMapper>();
    editorCamRotateIpt.mappers[1].clear();

    newProject();
}

void Editor::onUpdate()
{
    if (m_projectNeedReload)
    {
        reloadProject();
        m_projectNeedReload = false;
    }

    m_vpFrameBuff.onUpdate();
    processDroppedFiles();

    if (m_game && m_game->isRunning())
    {
        ECSView<ScriptComponent>(m_game->activeScene().ecsWorld()).onEach([&](Entity entt, ScriptComponent& scriptComponent){
            if (scriptComponent.instance)
                scriptComponent.instance->onUpdate();
        });
    }

    if (m_game && m_game->isRunning() == false)
        m_game.clear();

    if (!m_game)
    {
        assert(m_editedScene != nullptr);
        renderer().beginScene(m_editorCamera.getRendererCam(), m_vpFrameBuff);
        {
            renderer().addRenderables(m_editedScene->ecsWorld(), m_editedScene->assetManager());
            renderer().addLights(m_editedScene->ecsWorld());
        }
        renderer().endScene();

        addInputContext(&m_editorInputContext);
        setDispatchedInputContext(&m_editorInputContext);
    }
    else
    {
        assert(m_game->activeScene().activeCamera());
        assert(m_game->activeScene().activeCamera().has<TransformComponent>());
        assert(m_game->activeScene().activeCamera().has<CameraComponent>());

        Renderer::Camera rendererCamera = {
            m_game->activeScene().activeCamera().worldTransform_noScale().inversed(),
            m_game->activeScene().activeCamera().get<CameraComponent>().projectionMatrix()
        };
        
        renderer().beginScene(rendererCamera, m_vpFrameBuff);
        {
            renderer().addRenderables(m_game->activeScene().ecsWorld(), m_editedScene->assetManager());
            renderer().addLights(m_game->activeScene().ecsWorld());
        }
        renderer().endScene();

        addInputContext(&m_editorInputContext);
        addInputContext(&m_game->inputContext());
        setDispatchedInputContext(&m_game->inputContext());
    }
}

void Editor::onImGuiRender()
{
    static bool isProjectPropertiesModalPresented = false;
    static bool isFileOpenDialogPresented = false;
    static bool isFileSaveDialogPresented = false;

    ImGui::BeginDisabled(isFileOpenDialogPresented || isFileSaveDialogPresented);

    ImGui::DockSpaceOverViewport();

    MainMenuBar()
        .on_File_New(utils::Func<void()>(*this, &Editor::newProject))
        .on_File_Open([](){ isFileOpenDialogPresented = true; })
        .on_File_Save(m_projectSavePath.empty() ? [&](){ (void)(isFileSaveDialogPresented = true); } : utils::Func<void()>(*this, &Editor::saveProject))
        .on_Project_ReloadScriptLib(!(m_game && m_game->isRunning()) && !m_project.scriptLib().empty() ? utils::Func<void()>(*this, &Editor::reloadScriptLib) : utils::Func<void()>())
        .on_Project_Properties([](){ isProjectPropertiesModalPresented = true; })
        .on_Scene_Add_EmptyEntity([&](){ m_editedScene->newEntity("new_empty_entity"); })
        .on_Project_Run(!(m_game && m_game->isRunning()) ? utils::Func<void()>(*this, &Editor::runGame) : utils::Func<void()>())
        .on_Project_Stop((m_game && m_game->isRunning()) ? utils::Func<void()>(*this, &Editor::stopGame) : utils::Func<void()>())
        .render();

    ViewportPanel(*static_cast<const utils::SharedPtr<gfx::FrameBuffer>&>(m_vpFrameBuff)->colorTexture())
        .onResize(utils::Func<void(utils::uint32, utils::uint32)>(m_vpFrameBuff, &ViewportFrameBuff::resize))
        .render();

    SceneGraphPanel(m_editedScene, m_selectedEntity)
        .onEntitySelect([&](const Entity& e){ m_selectedEntity = e; })
        .render();

    EntityInspectorPanel(m_editedScene, m_selectedEntity)
        .onEntityDelete([&](){
            m_selectedEntity.destroy();
            m_selectedEntity = Entity();
        })
        .render();

    ProjectPropertiesModal(isProjectPropertiesModalPresented, m_project, m_projectSavePath)
        .onOk(utils::Func<void()>(*this, &Editor::reloadScriptLib))
        .render();

    ContentBrowserPanel(m_project, m_editedScene, m_getScriptNames)
        .render();

    ImGui::EndDisabled();

    FileOpenDialog("Open project", isFileOpenDialogPresented)
        .onSelection(utils::Func<void(const fs::path&)>(*this, &Editor::openProject))
        .render();

    FileSaveDialog(m_project.name() + ".geproj", isFileSaveDialogPresented)
        .onSelection([&](const fs::path& path){
            m_projectSavePath = path;
            saveProject();
        })
        .render();

    if (ImGui::GetIO().WantSaveIniSettings)
    {
        m_project.saveIniSettingsToMemory();
        ImGui::GetIO().WantSaveIniSettings = false;
    }

    if (ImGui::GetIO().WantCaptureKeyboard)
        setDispatchedInputContext(nullptr);
}

void Editor::onWindowRequestCloseEvent(gfx::WindowRequestCloseEvent&)
{
    terminate();
}

void Editor::newProject()
{
    m_projectSavePath = fs::path();
    m_projectNeedReload = true;
}

void Editor::openProject(const fs::path& filePath)
{
    assert(fs::is_regular_file(filePath));
    assert(filePath.is_absolute());
    
    m_projectSavePath = filePath;
    m_projectNeedReload = true;
}

void Editor::reloadProject()
{
    if (m_projectSavePath.empty())
        m_project = Project();
    else
    {
        assert(fs::is_regular_file(m_projectSavePath));
        assert(m_projectSavePath.is_absolute());
        m_project = json::parse(std::ifstream(m_projectSavePath));
    }

    reloadScriptLib();

    m_editedScene = nullptr;
    editScene(m_project.startScene());

    m_project.loadIniSettingsFromMemory();
}

void Editor::saveProject()
{
    assert(fs::is_directory(fs::path(m_projectSavePath).remove_filename()));
    std::ofstream(m_projectSavePath) << json(m_project).dump(4);
}

void Editor::reloadScriptLib()
{
    if (m_scriptLibHandle != nullptr)
    {
        dlFree(m_scriptLibHandle);
        m_scriptLibHandle = nullptr;
    }

    if (m_project.scriptLib().empty() == false)
    {
        fs::path scriptLibPath = fs::path(m_projectSavePath).remove_filename() / m_project.scriptLib();
        assert(scriptLibPath.is_absolute());
        assert(fs::is_regular_file(scriptLibPath));
        m_scriptLibHandle = dlLoad(scriptLibPath.c_str());
        if (m_scriptLibHandle != nullptr)
        {
            m_getScriptNames = (GetScriptNamesFn)getSym(m_scriptLibHandle, "getScriptNames");
            m_makeScriptInstance = (MakeScriptInstanceFn)getSym(m_scriptLibHandle, "makeScriptInstance");
            if (m_getScriptNames == nullptr || m_makeScriptInstance == nullptr)
            {
                dlFree(m_scriptLibHandle);
                m_scriptLibHandle = nullptr;
            }
        }
    }
}

void Editor::editScene(Scene* scene)
{
    if (m_editedScene)
    {
        assert(m_editedScene->assetManager().isLoaded());
        m_editedScene->assetManager().unloadAssets();
    }
    
    m_editedScene = scene;
    m_editedScene->assetManager().loadAssets(renderer().graphicAPI(), fs::path(m_projectSavePath).remove_filename());
    
    m_selectedEntity = Entity();
    m_editorCamera = EditorCamera();
}

void Editor::runGame()
{
    Game::Descriptor gameDescriptor;
    gameDescriptor.scenes = m_project.scenes();
    gameDescriptor.inputContext = m_project.inputContext();
    gameDescriptor.graphicAPI = &renderer().graphicAPI();
    gameDescriptor.baseDir = m_projectSavePath.remove_filename();
    gameDescriptor.makeScriptInstance = m_makeScriptInstance;

    m_game = utils::makeUnique<Game>(gameDescriptor);
    m_game->setActiveScene(m_project.startScene()->name());
}

void Editor::stopGame()
{
    m_game->stop();
}

Editor::~Editor()
{
    if (m_scriptLibHandle != nullptr)
    {
        dlFree(m_scriptLibHandle);
        m_scriptLibHandle = nullptr;
    }
}

void Editor::processDroppedFiles()
{
    fs::path path;
    while (window().popDroppedFile(path))
    {
        if (json::accept(std::ifstream(path)))
        {
            json jsn = json::parse(std::ifstream(path));
            if (jsn.find("scenes") != jsn.end() && jsn.find("imguiSettings") != jsn.end())
            {
                openProject(path);
                continue;
            }
        }

        if (stbi_info(path.c_str(), nullptr, nullptr, nullptr) == 1)
            continue;

        if (m_editedScene != nullptr)
        {
            m_editedScene->assetManager().registerMesh(path);
            continue;
        }
    }
}

}