/*
 * ---------------------------------------------------
 * Project.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/10 11:16:48
 * ---------------------------------------------------
 */

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "Scene.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"
#include <nlohmann/json.hpp>

#define DEFAULT_IMGUI_INI\
    "[Window][WindowOverViewport_11111111]\n"\
    "Pos=0,19\n"\
    "Size=1280,701\n"\
    "Collapsed=0\n"\
"\n"\
    "[Window][Debug##Default]\n"\
    "Pos=60,60\n"\
    "Size=400,400\n"\
    "Collapsed=0\n"\
"\n"\
    "[Window][Entity inspector]\n"\
    "Pos=999,494\n"\
    "Size=281,226\n"\
    "Collapsed=0\n"\
    "DockId=0x00000004,0\n"\
"\n"\
    "[Window][Scene graph]\n"\
    "Pos=999,19\n"\
    "Size=281,473\n"\
    "Collapsed=0\n"\
    "DockId=0x00000003,0\n"\
"\n"\
    "[Window][viewport]\n"\
    "Pos=0,19\n"\
    "Size=997,701\n"\
    "Collapsed=0\n"\
    "DockId=0x00000001,0\n"\
"\n"\
    "[Window][Project properties]\n"\
    "Pos=423,248\n"\
    "Size=337,287\n"\
    "Collapsed=0\n"\
"\n"\
    "[Docking][Data]\n"\
    "DockSpace     ID=0x7C6B3D9B Window=0xA87D555D Pos=942,394 Size=1280,701 Split=X Selected=0x0BA3B4F3\n"\
    "DockNode    ID=0x00000001 Parent=0x7C6B3D9B SizeRef=1185,701 CentralNode=1 Selected=0x0BA3B4F3\n"\
    "DockNode    ID=0x00000002 Parent=0x7C6B3D9B SizeRef=281,701 Split=Y Selected=0xF5BE1C77\n"\
        "DockNode  ID=0x00000003 Parent=0x00000002 SizeRef=168,473 Selected=0xF5BE1C77\n"\
        "DockNode  ID=0x00000004 Parent=0x00000002 SizeRef=168,226 Selected=0xD3D12213\n"

namespace GE
{

class Project
{
public:
    Project();
    Project(const Project&) = delete;
    Project(Project&&)      = default;
    
    Project(const utils::String& filepath);

    inline const utils::String& path() const { return m_path; }
    inline void setPath(const utils::String& path) { m_path = path; }
    
    inline const utils::String& name() const { return m_projName; }
    inline void setName(const utils::String& name) { m_projName = name; }

    void reloadProject();
    void saveProject();

    inline bool iniSettingsNeedLoad() const { return m_iniSettingsNeedLoad; }
    void loadIniSettings();

    inline void setRessourceDir(const utils::String& s) { m_ressourcesDir = s; }
    inline utils::String ressourceDirFullPath() const { return m_path + "/" + m_ressourcesDir; }

    virtual ~Project() = default;

private:
    utils::String m_path = "";

    utils::String m_projName = "new_project";
    utils::String m_ressourcesDir = "/";
    utils::String m_imguiIni = DEFAULT_IMGUI_INI;
    bool m_iniSettingsNeedLoad = false;

    utils::Set<Scene> m_scenes;
    Scene* m_startScene = nullptr;
    
public:
    Project& operator = (const Project&) = delete;
    Project& operator = (Project&&)      = default;
};

}

#endif // PROJECT_HPP