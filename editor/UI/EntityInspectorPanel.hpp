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

#include "Game-Engine/Entity.hpp"

namespace GE_Editor
{

class EntityInspectorPanel
{
public:
    EntityInspectorPanel() = delete;
    EntityInspectorPanel(const EntityInspectorPanel&) = delete;
    EntityInspectorPanel(EntityInspectorPanel&&) = delete;

    EntityInspectorPanel(const GE::Entity&);

    void render();

    ~EntityInspectorPanel() = default;

private:
    template<typename T> bool componentEditHeader(const char*);
    template<typename T> void componentEditWidget();
    void addComponentPopUp();

    GE::Entity m_entity;

public:
    EntityInspectorPanel& operator=(const EntityInspectorPanel&) = delete;
    EntityInspectorPanel& operator=(EntityInspectorPanel&&) = delete;
};

} // namespace GE_Editor

#endif // ENTITYINSPECTORPANEL_HPP
