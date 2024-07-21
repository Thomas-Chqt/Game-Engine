/*
 * ---------------------------------------------------
 * ECS_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 11:08:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "Game-Engine/ECSWorld.hpp"

TEST(ECSTest, entities)
{
    GE::ECSWorld world;

    GE::Entity entity1 = world.createEntity();
    EXPECT_EQ(entity1, 0);

    GE::Entity entity2 = world.createEntity();
    EXPECT_EQ(entity2, 1);
    
    world.deleteEntity(entity1);

    GE::Entity entity3 = world.createEntity();
    EXPECT_EQ(entity3, 0);
}