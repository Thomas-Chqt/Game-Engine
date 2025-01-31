/*
 * ---------------------------------------------------
 * ContentBrowserPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/06 14:16:00
 * ---------------------------------------------------
 */

#ifndef CONTENTBROWSERPANEL_HPP
#define CONTENTBROWSERPANEL_HPP

#include "Project.hpp"
#include "Scene.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include "Script.hpp"

namespace GE
{

class ContentBrowserPanel
{
public:
    ContentBrowserPanel()                           = delete;
    ContentBrowserPanel(const ContentBrowserPanel&) = delete;
    ContentBrowserPanel(ContentBrowserPanel&&)      = delete;
    
    ContentBrowserPanel(Project&, const Scene*, GetScriptNamesFn);

    void render();

    ~ContentBrowserPanel() = default;

private:
    void renderElement(const utils::String& name, const char *payloadType, const void* payload, utils::uint64 payloadSize);

    void renderScenes();
    void renderAssets();
    void renderScripts();

    Project& m_project;
    const Scene* m_scene;
    GetScriptNamesFn m_getScriptNames;

    float m_lineWith = 0.0F;

public:
    ContentBrowserPanel& operator = (const ContentBrowserPanel&) = delete;
    ContentBrowserPanel& operator = (ContentBrowserPanel&&)      = delete;
};

}

#endif // CONTENTBROWSERPANEL_HPP