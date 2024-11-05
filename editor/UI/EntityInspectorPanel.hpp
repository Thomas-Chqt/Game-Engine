/*
 * ---------------------------------------------------
 * EntityInspectorPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/04 15:03:54
 * ---------------------------------------------------
 */

#ifndef ENTITYINSPECTORPANEL_HPP
#define ENTITYINSPECTORPANEL_HPP

#include "ECS/Entity.hpp"
#include "Scene.hpp"
#include "UtilsCPP/Func.hpp"

namespace GE
{

class EntityInspectorPanel
{
public:
    EntityInspectorPanel()                            = delete;
    EntityInspectorPanel(const EntityInspectorPanel&) = delete;
    EntityInspectorPanel(EntityInspectorPanel&&)      = delete;
    
    EntityInspectorPanel(Scene*, const Entity&);

    inline EntityInspectorPanel& onEntityDelete(const utils::Func<void()>& f) { return m_onEntityDelete = f, *this; }
    inline EntityInspectorPanel& onEntityDelete(utils::Func<void()>&& f) { return m_onEntityDelete = std::move(f), *this; }

    void render();

    ~EntityInspectorPanel() = default;

private:
    template<typename T> bool componentEditHeader(const char*);
    template<typename T> void componentEditWidget();
    void addComponentPopUp();

    Scene* m_scene;
    Entity m_selectedEntity;

    utils::Func<void()> m_onEntityDelete;


public:
    EntityInspectorPanel& operator = (const EntityInspectorPanel&) = delete;
    EntityInspectorPanel& operator = (EntityInspectorPanel&&)      = delete;
};

}

#endif // ENTITYINSPECTORPANEL_HPP