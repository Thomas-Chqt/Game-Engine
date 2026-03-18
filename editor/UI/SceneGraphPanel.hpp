/*
 * ---------------------------------------------------
 * SceneGraphPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/01 16:45:57
 * ---------------------------------------------------
 */

#ifndef SCENEGRAPHPANEL_HPP
#define SCENEGRAPHPANEL_HPP

#include <Game-Engine/ECSWorld.hpp>
#include <Game-Engine/Scene.hpp>
#include <Game-Engine/Entity.hpp>

#include <functional>

namespace GE_Editor
{

class SceneGraphPanel
{
public:
    SceneGraphPanel() = delete;
    SceneGraphPanel(const SceneGraphPanel&) = delete;
    SceneGraphPanel(SceneGraphPanel&&) = delete;

    SceneGraphPanel(GE::Scene*, GE::Entity*);

    void render();

    ~SceneGraphPanel() = default;

private:
    void renderEntityRow(GE::Entity&);

    GE::Scene* m_scene = nullptr;
    GE::Entity* m_selectedEntity = nullptr;

    std::function<void(GE::Entity& droped, GE::Entity& newParent)> m_onEntityDrop;

public:
    SceneGraphPanel& operator=(const SceneGraphPanel&) = delete;
    SceneGraphPanel& operator=(SceneGraphPanel&&) = delete;
};

} // namespace GE_Editor

#endif // SCENEGRAPHPANEL_HPP
