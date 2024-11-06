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

#include "UtilsCPP/Func.hpp"
#include <utility>

namespace GE
{

class MainMenuBar
{
public:
    MainMenuBar()                   = default;
    MainMenuBar(const MainMenuBar&) = delete;
    MainMenuBar(MainMenuBar&&)      = delete;
    
    inline MainMenuBar& on_File_New(const utils::Func<void()>& f) { return m_file.neew = f, *this; }
    inline MainMenuBar& on_File_New(utils::Func<void()>&& f) { return m_file.neew = std::move(f), *this; }

    inline MainMenuBar& on_File_Open(const utils::Func<void()>& f) { return m_file.open = f, *this; }
    inline MainMenuBar& on_File_Open(utils::Func<void()>&& f) { return m_file.open = std::move(f), *this; }

    inline MainMenuBar& on_File_Save(const utils::Func<void()>& f) { return m_file.save = f, *this; }
    inline MainMenuBar& on_File_Save(utils::Func<void()>&& f) { return m_file.save = std::move(f), *this; }

    inline MainMenuBar& on_Project_Properties(const utils::Func<void()>& f) { return m_project.properties = f, *this; }
    inline MainMenuBar& on_Project_Properties(utils::Func<void()>&& f) { return m_project.properties = std::move(f), *this; }

    inline MainMenuBar& on_Project_ReloadScriptLib(const utils::Func<void()>& f) { return m_project.reloadScriptLib = f, *this; }
    inline MainMenuBar& on_Project_ReloadScriptLib(utils::Func<void()>&& f) { return m_project.reloadScriptLib = std::move(f), *this; }

    inline MainMenuBar& on_Project_Run(const utils::Func<void()>& f) { return m_project.run = f, *this; }
    inline MainMenuBar& on_Project_Run(utils::Func<void()>&& f) { return m_project.run = std::move(f), *this; }

    inline MainMenuBar& on_Project_Stop(const utils::Func<void()>& f) { return m_project.stop = f, *this; }
    inline MainMenuBar& on_Project_Stop(utils::Func<void()>&& f) { return m_project.stop = std::move(f), *this; }

    inline MainMenuBar& on_Debug_ShowDemoWindow(const utils::Func<void()>& f) { return m_debug.showDemoWindow = f, *this; }
    inline MainMenuBar& on_Debug_ShowDemoWindow(utils::Func<void()>&& f) { return m_debug.showDemoWindow = std::move(f), *this; }

    inline MainMenuBar& on_Debug_ShowMetricsWindow(const utils::Func<void()>& f) { return m_debug.showMetricsWindow = f, *this; }
    inline MainMenuBar& on_Debug_ShowMetricsWindow(utils::Func<void()>&& f) { return m_debug.showMetricsWindow = std::move(f), *this; }

    inline MainMenuBar& on_Debug_StartEditedScene(const utils::Func<void()>& f) { return m_debug.startEditedScene = f, *this; }
    inline MainMenuBar& on_Debug_StartEditedScene(utils::Func<void()>&& f) { return m_debug.startEditedScene = std::move(f), *this; }

    inline MainMenuBar& on_Debug_StopEditedScene(const utils::Func<void()>& f) { return m_debug.stopEditedScene = f, *this; }
    inline MainMenuBar& on_Debug_StopEditedScene(utils::Func<void()>&& f) { return m_debug.stopEditedScene = std::move(f), *this; }

    void render();

    ~MainMenuBar() = default;

private:
    struct
    {
        utils::Func<void()> neew;
        utils::Func<void()> open;
        utils::Func<void()> save;
    } m_file;

    struct
    {
        utils::Func<void()> properties;
        utils::Func<void()> reloadScriptLib;
        utils::Func<void()> run;
        utils::Func<void()> stop;
    } m_project;

    struct
    {
        utils::Func<void()> showDemoWindow;
        utils::Func<void()> showMetricsWindow;
        utils::Func<void()> startEditedScene;
        utils::Func<void()> stopEditedScene;
    } m_debug;

public:
    MainMenuBar& operator = (const MainMenuBar&) = delete;
    MainMenuBar& operator = (MainMenuBar&&)      = delete;
};

}

#endif // MAINMENUBAR_HPP