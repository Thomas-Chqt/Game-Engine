/*
 * ---------------------------------------------------
 * MainMenuBar.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/28 18:39:45
 * ---------------------------------------------------
 */

#ifndef MAINMENUBAR_HPP
#define MAINMENUBAR_HPP

#include <functional>
#include <utility>

namespace GE_Editor
{

class MainMenuBar
{
public:
    MainMenuBar() = default;
    MainMenuBar(const MainMenuBar&) = delete;
    MainMenuBar(MainMenuBar&&) = delete;

    inline MainMenuBar& on_File_New(std::function<void()>&& f) { return m_file.neew = std::move(f), *this; }
    inline MainMenuBar& on_File_Open(std::function<void()>&& f) { return m_file.open = std::move(f), *this; }
    inline MainMenuBar& on_File_Save(std::function<void()>&& f) { return m_file.save = std::move(f), *this; }

    inline MainMenuBar& on_Project_Properties(std::function<void()>&& f) { return m_project.properties = std::move(f), *this; }
    inline MainMenuBar& on_Project_ReloadScriptLib(std::function<void()>&& f) { return m_project.reloadScriptLib = std::move(f), *this; }
    inline MainMenuBar& on_Project_Run(std::function<void()>&& f) { return m_project.run = std::move(f), *this; }
    inline MainMenuBar& on_Project_Stop(std::function<void()>&& f) { return m_project.stop = std::move(f), *this; }

    inline MainMenuBar& on_Scene_Add_EmptyEntity(std::function<void()>&& f) { return m_scene.neew.emptyEntity = std::move(f), *this; }

    inline MainMenuBar& on_Debug_ShowDemoWindow(std::function<void()>&& f) { return m_debug.showDemoWindow = std::move(f), *this; }
    inline MainMenuBar& on_Debug_ShowMetricsWindow(std::function<void()>&& f) { return m_debug.showMetricsWindow = std::move(f), *this; }
    inline MainMenuBar& on_Debug_StartEditedScene(std::function<void()>&& f) { return m_debug.startEditedScene = std::move(f), *this; }
    inline MainMenuBar& on_Debug_StopEditedScene(std::function<void()>&& f) { return m_debug.stopEditedScene = std::move(f), *this; }

    void render();

    ~MainMenuBar() = default;

private:
    struct
    {
        std::function<void()> neew;
        std::function<void()> open;
        std::function<void()> save;
    } m_file;

    struct
    {
        std::function<void()> properties;
        std::function<void()> reloadScriptLib;
        std::function<void()> run;
        std::function<void()> stop;
    } m_project;

    struct
    {
        struct
        {
            std::function<void()> emptyEntity;
        } neew;
    } m_scene;

    struct
    {
        std::function<void()> showDemoWindow;
        std::function<void()> showMetricsWindow;
        std::function<void()> startEditedScene;
        std::function<void()> stopEditedScene;
    } m_debug;

public:
    MainMenuBar& operator=(const MainMenuBar&) = delete;
    MainMenuBar& operator=(MainMenuBar&&) = delete;
};

} // namespace GE

#endif // MAINMENUBAR_HPP
