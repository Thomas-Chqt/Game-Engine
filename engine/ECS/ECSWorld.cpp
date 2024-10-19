/*
 * ---------------------------------------------------
 * ECSWorld.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 09:35:25
 * ---------------------------------------------------
 */

#include "ECS/ECSWorld.hpp"
#include "ECS/Components.hpp"
#include "ECS/Entity.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/Types.hpp"
#include <cassert>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace GE
{

ECSWorld::ECSWorld()
{
    m_archetypes.insert(ArchetypeID{0}, Archetype());
}

ECSWorld::EntityID ECSWorld::newEntityID()
{
    // new entity has no component so directly inserting in empty archetype (the one with only the entity id)
    ArchetypeID newEntityArcId = { 0 };
    Archetype& newEntityArchetype = m_archetypes[newEntityArcId];
    utils::uint64 newEntityIdx = newEntityArchetype.allocateCollum();

    EntityID newEntityId;
    if (m_availableEntityIDs.isEmpty())
    {
        newEntityId = m_entityDatas.length();
        m_entityDatas.append(EntityData{newEntityArcId, newEntityIdx});
    }
    else
    {
        newEntityId = m_availableEntityIDs.pop(m_availableEntityIDs.begin());
        m_entityDatas[newEntityId] = EntityData{newEntityArcId, newEntityIdx};
    }
    newEntityArchetype.getEntityID(newEntityIdx) = newEntityId;

    assert(isValidEntityID(newEntityId));
    return newEntityId;
}

void ECSWorld::deleteEntityID(EntityID entityId)
{
    assert(isValidEntityID(entityId));
    Archetype& entityArch = m_archetypes[m_entityDatas[entityId].archetypeId];
    utils::uint64 entityIdx = m_entityDatas[entityId].idx;

    // last entity of the archetype will be move to the index of the delete entity
    // so the idx in the entity datas need to be change
    m_entityDatas[entityArch.getEntityID(entityArch.size() - 1)].idx = entityIdx;

    entityArch.destructCollum(entityIdx);
    Archetype::moveComponents(entityArch, entityArch.size() - 1, entityArch, entityIdx);
    entityArch.destructCollum(entityArch.size() - 1);
    entityArch.freeLastCollum();

    m_availableEntityIDs.insert(entityId);
}

utils::uint32 ECSWorld::componentCount()
{
    utils::uint64 count = 0;
    for (auto& [id, arch] : m_archetypes)
        count += (id.size() - 1) * arch.size();
    return count;
}

ECSWorld::Iterator ECSWorld::begin() const
{
    EntityID id = 0;
    while (m_availableEntityIDs.contain(id))
        id++;
    return Iterator(*this, id);
}

ECSWorld::Iterator ECSWorld::end() const
{
    return Iterator(*this, m_entityDatas.length());
}

ECSWorld::ComponentID ECSWorld::nextComponentID()
{
    // start at 1 because 0 is reserved for the entity id
    static ComponentID id = 1;
    return id++;
};

void to_json(nlohmann::json& jsn, const ECSWorld& world)
{
    jsn["entities"] = json::array();

    for (auto id : world)
    {
        json entityJsn;
        entityJsn["id"] = id;

        if (world.has<NameComponent>(id))
        {
            const NameComponent& comp = world.get<NameComponent>(id);
            entityJsn["nameComponent"] = {
                {"name", comp.name}
            };
        }

        if (world.has<HierarchyComponent>(id))
        {
            const HierarchyComponent& comp = world.get<HierarchyComponent>(id);
            entityJsn["hierarchyComponent"] = {
                { "parent", comp.parent.entityID() },
                { "firstChild", comp.firstChild.entityID() },
                { "nextChild", comp.nextChild.entityID() }
            };
        }

        if (world.has<TransformComponent>(id))
        {
            const TransformComponent& comp = world.get<TransformComponent>(id);
            entityJsn["transformComponent"] = {
                { "position", {
                    {"x", comp.position.x },
                    {"y", comp.position.y },
                    {"z", comp.position.z }
                }},
                { "rotation", {
                    {"x", comp.rotation.x },
                    {"y", comp.rotation.y },
                    {"z", comp.rotation.z }
                }},
                { "scale", {
                    {"x", comp.scale.x },
                    {"y", comp.scale.y },
                    {"z", comp.scale.z }
                }}
            };
        }

        if (world.has<CameraComponent>(id))
        {
            const CameraComponent& comp = world.get<CameraComponent>(id);
            entityJsn["cameraComponent"] = {
                { "fov", comp.fov },
                { "zFar", comp.zFar },
                { "zNear", comp.zNear }
            };
        }

        if (world.has<LightComponent>(id))
        {
            const LightComponent& comp = world.get<LightComponent>(id);
            entityJsn["lightComponent"] = {
                { "type", (utils::uint8)comp.type },
                { "color", {
                    {"r", comp.color.r },
                    {"g", comp.color.g },
                    {"b", comp.color.b }
                }},
                { "intentsity", comp.intentsity }
            };
        }

        jsn["entities"].emplace_back(entityJsn);
    }

    jsn["availableEntityIDJsn"] = json::array();

    for (auto id : world.m_availableEntityIDs)
        jsn["availableEntityIDJsn"].emplace_back(id);
}

void from_json(const nlohmann::json& jsn, ECSWorld& world)
{
    world.m_entityDatas = utils::Array<ECSWorld::EntityData>(jsn["entities"].size() + jsn["availableEntityIDJsn"].size());

    for (auto& entity : jsn["entities"])
    {
        ECSWorld::EntityID entityId = entity["id"].template get<ECSWorld::EntityID>();
        ECSWorld::ArchetypeID newEntityArcId = { 0 };
        ECSWorld::Archetype& newEntityArchetype = world.m_archetypes[newEntityArcId];
        utils::uint64 newEntityIdx = newEntityArchetype.allocateCollum();
        world.m_entityDatas[entityId] = ECSWorld::EntityData{newEntityArcId, newEntityIdx};
        newEntityArchetype.getEntityID(newEntityIdx) = entityId;
        
        auto nameComponentIt = entity.find("nameComponent");
        if (nameComponentIt != entity.end())
        {
            world.emplace<NameComponent>(entityId,
                (*nameComponentIt)["name"].template get<std::string>().c_str()
            );
        }

        auto hierarchyComponentIt = entity.find("hierarchyComponent");
        if (hierarchyComponentIt != entity.end())
        {
            HierarchyComponent& comp = world.emplace<HierarchyComponent>(entityId);
            comp.parent = Entity(world, (*hierarchyComponentIt)["parent"].template get<ECSWorld::EntityID>());
            comp.firstChild = Entity(world, (*hierarchyComponentIt)["firstChild"].template get<ECSWorld::EntityID>());
            comp.nextChild = Entity(world, (*hierarchyComponentIt)["nextChild"].template get<ECSWorld::EntityID>());
        }

        auto transformComponentIt = entity.find("transformComponent");
        if (hierarchyComponentIt != entity.end())
        {
            world.emplace<TransformComponent>(entityId,
                math::vec3f(
                    (*transformComponentIt)["position"]["x"].template get<float>(),
                    (*transformComponentIt)["position"]["y"].template get<float>(),
                    (*transformComponentIt)["position"]["z"].template get<float>()
                ),
                math::vec3f(
                    (*transformComponentIt)["rotation"]["x"].template get<float>(),
                    (*transformComponentIt)["rotation"]["y"].template get<float>(),
                    (*transformComponentIt)["rotation"]["z"].template get<float>()
                ),
                math::vec3f(
                    (*transformComponentIt)["scale"]["x"].template get<float>(),
                    (*transformComponentIt)["scale"]["y"].template get<float>(),
                    (*transformComponentIt)["scale"]["z"].template get<float>()
                )
            );
        }

        auto cameraComponentIt = entity.find("cameraComponent");
        if (cameraComponentIt != entity.end())
        {
            world.emplace<CameraComponent>(entityId,
                (*cameraComponentIt)["fov"].template get<float>(),
                (*cameraComponentIt)["zFar"].template get<float>(),
                (*cameraComponentIt)["zNear"].template get<float>()
            );
        }

        auto lightComponentIt = entity.find("lightComponent");
        if (lightComponentIt != entity.end())
        {
            world.emplace<LightComponent>(entityId,
                (LightComponent::Type)((*lightComponentIt)["type"].template get<utils::uint8>()),
                math::rgb(
                    (*transformComponentIt)["color"]["r"].template get<float>(),
                    (*transformComponentIt)["color"]["g"].template get<float>(),
                    (*transformComponentIt)["color"]["b"].template get<float>()
                ),
                (*lightComponentIt)["intentsity"].template get<float>()
            );
        }
    }

    for (auto& id : jsn["availableEntityIDJsn"])
        world.m_availableEntityIDs.insert(id);
}

}