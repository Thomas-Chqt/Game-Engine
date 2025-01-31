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

#include "Scene.hpp"
#include "UtilsCPP/Func.hpp"
#include "ECS/Entity.hpp"
#include <utility>

namespace GE
{

class SceneGraphPanel
{
public:
    SceneGraphPanel()                       = delete;
    SceneGraphPanel(const SceneGraphPanel&) = delete;
    SceneGraphPanel(SceneGraphPanel&&)      = delete;
    
    SceneGraphPanel(Scene*, const Entity&);

    inline SceneGraphPanel& onEntitySelect(const utils::Func<void(const Entity&)>& f) { return m_onEntitySelect = f, *this; }
    inline SceneGraphPanel& onEntitySelect(utils::Func<void(const Entity&)>&& f) { return m_onEntitySelect = std::move(f), *this; }

    void render();

    ~SceneGraphPanel() = default;

private:
    void renderEntityRow(Entity&);
    void entityDndTarget(Entity parent = Entity());

    Scene* m_scene;
    Entity m_selectedEntity;

    utils::Func<void(const Entity&)> m_onEntitySelect;

public:
    SceneGraphPanel& operator = (const SceneGraphPanel&) = delete;
    SceneGraphPanel& operator = (SceneGraphPanel&&)      = delete;
};

}

#endif // SCENEGRAPHPANEL_HPP