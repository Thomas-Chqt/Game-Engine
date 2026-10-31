#pragma once

#include <Game-Engine/Components.hpp>

#include <yaml-cpp/yaml.h>

#include <array>
#include <charconv>
#include <limits>
#include <span>

namespace YAML
{


template<>
struct convert<GE::EntityID>
{
    static Node encode(const GE::EntityID& id) { return Node(uuids::to_string(id)); }

    static bool decode(const Node& node, GE::EntityID& id)
    {
        if (!node.IsScalar())
            return false;
        const auto& text = node.Scalar();
        if (auto parsed = GE::EntityID::from_string(text)) {
            if (parsed->is_nil())
                return false;
            id = *parsed;
            return true;
        }
        // Old projects use uint64 entity IDs. Embed them deterministically so
        // entity declarations, hierarchy links and editor selection still agree.
        uint64_t legacyId = 0;
        auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), legacyId);
        if (error != std::errc{} || end != text.data() + text.size())
            return false;
        if (legacyId == std::numeric_limits<uint64_t>::max())
            return false;
        std::array<uint8_t, 16> bytes{};
        bytes[0] = 1; // Keep legacy entity 0 distinct from the nil UUID.
        for (auto& byte : std::span(bytes).subspan<8>()) {
            byte = static_cast<uint8_t>(legacyId);
            legacyId >>= 8;
        }
        id = GE::EntityID(bytes);
        return true;
    }
};

template<>
struct convert<std::optional<GE::EntityID>>
{
    static Node encode(const std::optional<GE::EntityID>& id)
    {
        return id ? convert<GE::EntityID>::encode(*id) : Node(NodeType::Null);
    }

    static bool decode(const Node& node, std::optional<GE::EntityID>& id)
    {
        if (!node || node.IsNull()) {
            id.reset();
            return true;
        }
        // Accept previous project encodings of absent references only here.
        if (node.IsScalar()) {
            const auto& text = node.Scalar();
            const auto uuid = GE::EntityID::from_string(text);
            uint64_t legacyId = 0;
            const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), legacyId);
            if ((uuid && uuid->is_nil()) || (error == std::errc{} && end == text.data() + text.size()
                && legacyId == std::numeric_limits<uint64_t>::max())) {
                id.reset();
                return true;
            }
        }
        GE::EntityID value;
        if (!convert<GE::EntityID>::decode(node, value))
            return false;
        id = value;
        return true;
    }
};

template<>
struct convert<std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>>
{
    static Node encode(const std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>& rhs);
    static bool decode(const Node& node, std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>& rhs);
};

}
