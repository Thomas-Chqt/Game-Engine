#pragma once

#include "Game-Engine/ECSRegistry.hpp"

namespace GE
{

using ECSWorld = ECSRegistry;

template<typename T>
concept ECSWorldLike = ECSRegistryLike<T>;

}
