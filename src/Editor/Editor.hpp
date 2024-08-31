/*
 * ---------------------------------------------------
 * Editor.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/30 18:45:16
 * ---------------------------------------------------
 */

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "Application/Application.hpp"
#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Game.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/Window.hpp"
#include "Math/Vector.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class Editor : public Application
{
public:
    Editor()              = delete;
    Editor(const Editor&) = delete;
    Editor(Editor&&)      = delete;

    Editor(int argc, char* argv[]);

    inline bool shouldTerminate() { return m_shouldTerminate; }

    void onSetup();
    void onUpdate();

    ~Editor();

private:
    void onEvent(gfx::Event&);
    void onImGuiRender();

    // ImGuiPanels
    void drawViewportPanel();
    void drawSceneGraphPanel();
    void drawEntityInspectorPanel();
    void drawFPSPanel();

    bool m_shouldTerminate = true;

    utils::SharedPtr<gfx::Window> m_window;
    utils::SharedPtr<gfx::GraphicAPI> m_graphicAPI;
    Renderer m_renderer;

    utils::UniquePtr<Game> m_game;
    ECSWorld m_renderedScene;

    Entity m_selectedEntity;

    math::vec3f m_editorCameraPos;
    math::vec3f m_editorCameraRot;

    math::vec2f m_viewportPanelSize;
    utils::SharedPtr<gfx::FrameBuffer> m_viewportFBuff;


public:
    Editor& operator = (const Editor&) = delete;
    Editor& operator = (Editor&&)      = delete;
};

}

#endif // EDITOR_HPP