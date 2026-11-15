#pragma once

#include <Game-Engine/Entity.hpp>

#include <array>
#include <optional>
#include <vector>

namespace GE_Editor
{
struct ComponentInspectorState
{
    std::array<char, 96> search{};
    int highlighted = -1;
    std::optional<GE::Entity> previousEntity;
    int reveal = -1;
};

struct SceneGraphState
{
    std::optional<GE::Entity> entity;
    std::vector<char> buffer;
    bool focus = false;
    bool rendered = false;

    void commit();
    void begin(GE::Entity target);
};
} // namespace GE_Editor
