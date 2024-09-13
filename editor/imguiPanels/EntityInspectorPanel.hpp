/*
 * ---------------------------------------------------
 * EntityInspectorPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/13 16:32:26
 * ---------------------------------------------------
 */

#ifndef ENTITYINSPECTORPANEL_HPP
#define ENTITYINSPECTORPANEL_HPP

#include "ECS/Entity.hpp"
#include "Scene.hpp"

namespace GE
{

class EntityInspectorPanel
{
public:
    EntityInspectorPanel()                            = delete;
    EntityInspectorPanel(const EntityInspectorPanel&) = delete;
    EntityInspectorPanel(EntityInspectorPanel&&)      = delete;
    
    EntityInspectorPanel(Scene* scene, Entity selectedEntity);

    void render();

    ~EntityInspectorPanel() = default;

private:
    template<typename T>
    bool componentEditHeader(const char* title);

    void transformComponentEditWidget();
    void cameraComponentEditWidget();
    void lightComponentEditWidget();
    void meshComponentEditWidget();
    void addComponentPopup();
    
    Scene* m_editedScene;
    Entity m_selectedEntity;

public:
    EntityInspectorPanel& operator = (const EntityInspectorPanel&) = delete;
    EntityInspectorPanel& operator = (EntityInspectorPanel&&)      = delete;
};

}

#endif // ENTITYINSPECTORPANEL_HPP