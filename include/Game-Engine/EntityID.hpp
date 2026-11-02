#pragma once

#include <uuid.h>
#include <unordered_map>

namespace GE
{

using EntityID = uuids::uuid;
using EntityIDMap = std::unordered_map<EntityID, EntityID>;

}
