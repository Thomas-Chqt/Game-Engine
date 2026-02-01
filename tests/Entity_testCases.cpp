/*
 * ---------------------------------------------------
 * Entity_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2026/02/01 11:09:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Components.hpp"

namespace GE_tests
{

TEST(EntityTest, staticAssert)
{
    static_assert(GE::EntityLike<GE::Entity>, "Entity is not EntityLike");
    static_assert(!GE::EntityLike<const GE::Entity>, "const Entity is EntityLike");
    static_assert(GE::ConstEntityLike<GE::Entity>, "Entity is not ConstEntityLike");
    static_assert(GE::ConstEntityLike<const GE::Entity>, "const Entity is not ConstEntityLike");

    static_assert(!GE::EntityLike<GE::const_Entity>, "const_Entity is EntityLike");
    static_assert(!GE::EntityLike<const GE::const_Entity>, "const const_Entity is EntityLike");
    static_assert(GE::ConstEntityLike<GE::const_Entity>, "const_Entity is not ConstEntityLike");
    static_assert(GE::ConstEntityLike<const GE::const_Entity>, "const const_Entity is not ConstEntityLike");
}

TEST(EntityTest, emplaceAndGet)
{
    GE::ECSWorld ecsWorld;

    GE::Entity entity = {
        .world = &ecsWorld,
        .entityId = ecsWorld.newEntityID()
    };

    GE::CameraComponent& cameraComponent = entity.emplace<GE::CameraComponent>();
    cameraComponent.fov = 90.0f;

    const GE::Entity entity2 = entity;
    const GE::CameraComponent& cameraComponent2 = entity2.get<GE::CameraComponent>();
    ASSERT_FLOAT_EQ(cameraComponent2.fov, 90.0f);

    GE::const_Entity entity3 = entity;
    const GE::CameraComponent& cameraComponent3 = entity3.get<GE::CameraComponent>();
    ASSERT_FLOAT_EQ(cameraComponent3.fov, 90.0f);

    const GE::const_Entity entity4 = entity;
    const GE::CameraComponent& cameraComponent4 = entity4.get<GE::CameraComponent>();
    ASSERT_FLOAT_EQ(cameraComponent4.fov, 90.0f);
}

}
