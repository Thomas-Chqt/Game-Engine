/*
 * ---------------------------------------------------
 * SceneGraphPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/13 14:34:37
 * ---------------------------------------------------
 */

#ifndef SCENEGRAPHPANEL_HPP
#define SCENEGRAPHPANEL_HPP

#include "ECS/Entity.hpp"
#include "Scene.hpp"

namespace GE
{

class SceneGraphPanel
{
public:
    SceneGraphPanel(const SceneGraphPanel&) = delete;
    SceneGraphPanel(SceneGraphPanel&&)      = delete;

    SceneGraphPanel(Scene*, Entity selectedEntity);

    SceneGraphPanel& onEntitySelect(const utils::Func<void(Entity)>&);

    void render();

    ~SceneGraphPanel() = default;

private:
    void renderRow(Entity entity);

    Scene* m_scene;
    Entity m_selectedEntity;
    utils::Func<void(Entity)> m_onEntitySelect;

public:
    SceneGraphPanel& operator = (const SceneGraphPanel&) = delete;
    SceneGraphPanel& operator = (SceneGraphPanel&&)      = delete;
};

}

#endif // SCENEGRAPHPANEL_HPP