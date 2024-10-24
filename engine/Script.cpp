/*
 * ---------------------------------------------------
 * Script.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/24 12:43:05
 * ---------------------------------------------------
 */

#include "Script.hpp"
#include "ECS/Entity.hpp"

namespace GE
{

void Script::setEntity(Entity entt)
{
    m_entity = entt;
}

}