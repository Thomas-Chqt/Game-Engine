#include "convert_input.hpp"

#include <Game-Engine/InputContext.hpp>
#include <Game-Engine/Input.hpp>

#include <yaml-cpp/yaml.h>

#include <string>
#include <utility>

namespace YAML
{

template<GE::RawInput T> struct RawInputTraits;

template<> struct RawInputTraits<GE::KeyboardButton> { static constexpr std::string_view name = "KeyboardButton"; };
template<> struct RawInputTraits<GE::MouseButton>    { static constexpr std::string_view name = "MouseButton";    };

template<GE::InputType T> struct InputTraits;

template<> struct InputTraits<GE::ActionInput>  { static constexpr std::string_view name = "Action";  };
template<> struct InputTraits<GE::StateInput>   { static constexpr std::string_view name = "State";   };
template<> struct InputTraits<GE::RangeInput>   { static constexpr std::string_view name = "Range";   };
template<> struct InputTraits<GE::Range2DInput> { static constexpr std::string_view name = "Range2D"; };

template<>
struct convert<GE::KeyboardButton>
{
    static Node encode(const GE::KeyboardButton& rhs)
    {
        return Node(std::string(GE::keyboardButtonName(rhs)));
    }

    static bool decode(const Node& node, GE::KeyboardButton& rhs)
    {
        if (!node.IsScalar())
            return false;

        if (const std::optional<GE::KeyboardButton> button = GE::keyboardButtonFromName(node.as<std::string>()))
        {
            rhs = *button;
            return true;
        }
        return false;
    }
};

template<GE::InputType InputT>
struct convert<GE::InputMapper<GE::KeyboardButton, InputT>>
{
    static Node encode(const GE::InputMapper<GE::KeyboardButton, InputT>& rhs)
    {
        Node node;
        if constexpr (std::is_same_v<InputT, GE::Range2DInput>)
        {
            node["xPos"] = rhs.xPos;
            node["xNeg"] = rhs.xNeg;
            node["xScale"] = rhs.xScale;
            node["yPos"] = rhs.yPos;
            node["yNeg"] = rhs.yNeg;
            node["yScale"] = rhs.yScale;
            node["triggerValue"] = rhs.triggerValue;
        }
        else
        {
            node["button"] = rhs.button;
            if constexpr (std::is_same_v<InputT, GE::RangeInput>)
                node["scale"] = rhs.scale;
        }
        return node;
    }

    static bool decode(const Node& node, GE::InputMapper<GE::KeyboardButton, InputT>& rhs)
    {
        if (!node.IsMap())
            return false;

        if constexpr (std::is_same_v<InputT, GE::Range2DInput>)
        {
            if (!node["xPos"] || !node["xNeg"] || !node["yPos"] || !node["yNeg"])
                return false;

            rhs = GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
                .xPos = node["xPos"].as<GE::KeyboardButton>(),
                .xNeg = node["xNeg"].as<GE::KeyboardButton>(),
                .xScale = node["xScale"] ? node["xScale"].as<float>() : 1.0f,
                .yPos = node["yPos"].as<GE::KeyboardButton>(),
                .yNeg = node["yNeg"].as<GE::KeyboardButton>(),
                .yScale = node["yScale"] ? node["yScale"].as<float>() : 1.0f,
                .triggerValue = node["triggerValue"] ? node["triggerValue"].as<float>() : 0.5f });
        }
        else
        {
            if (!node["button"])
                return false;

            if constexpr (std::is_same_v<InputT, GE::RangeInput>)
                rhs = GE::InputMapper<GE::KeyboardButton, InputT>(node["button"].as<GE::KeyboardButton>(), node["scale"] ? node["scale"].as<float>() : 1.0f);
            else
                rhs = GE::InputMapper<GE::KeyboardButton, InputT>(node["button"].as<GE::KeyboardButton>());
        }

        return true;
    }
};

template<>
struct convert<GE::VInput>
{
    static Node encode(const GE::VInput& rhs)
    {
        return std::visit([](const auto& input) -> Node {
            Node node;
            using InputT = std::remove_cvref_t<decltype(input)>;
            node["type"] = std::string(InputTraits<InputT>::name);
            if (input.mapper)
            {
                Node mapperNode;
                std::visit([&](const auto& mapper) {
                    using MapperT = std::remove_cvref_t<decltype(mapper)>;
                    using RawInputT = typename MapperT::RawInputType;
                    mapperNode["type"] = std::string(RawInputTraits<RawInputT>::name);
                    mapperNode["data"] = mapper;
                }, *input.mapper);
                node["mapper"] = mapperNode;
            }
            return node;
        }, rhs);
    }

    static bool decode(const Node& node, GE::VInput& rhs)
    {
        if (!node.IsMap() || !node["type"])
            return false;

        auto type = node["type"].as<std::string>();
        return GE::anyOfType<GE::InputTypes>([&]<typename InputT>() {
            if (type != InputTraits<InputT>::name)
                return false;

            InputT input;
            if (node["mapper"])
            {
                const Node& mapperNode = node["mapper"];
                if (!mapperNode.IsMap() || !mapperNode["type"] || !mapperNode["data"])
                    return false;

                auto mapperType = mapperNode["type"].as<std::string>();
                const bool decoded = GE::anyOfType<GE::InputMapperTypes>([&]<typename MapperT>() {
                    if constexpr (!std::same_as<typename MapperT::InputType, InputT>)
                        return false;
                    else
                    {
                        using RawInputT = typename MapperT::RawInputType;
                        if (mapperType != RawInputTraits<RawInputT>::name)
                            return false;

                        input.setMapper(mapperNode["data"].as<MapperT>());
                        return true;
                    }
                });

                if (!decoded)
                    return false;
            }

            rhs = input;
            return true;
        });
    }
};

Node convert<std::pair<std::string, GE::VInput>>::encode(const std::pair<std::string, GE::VInput>& rhs)
{
    Node node;
    auto& [name, input] = rhs;
    node["name"] = name;
    node["input"] = input;
    return node;
}

bool convert<std::pair<std::string, GE::VInput>>::decode(const Node& node, std::pair<std::string, GE::VInput>& rhs)
{
    if (!node.IsMap() || !node["name"] || !node["input"])
        return false;

    auto& [name, input] = rhs;
    name = node["name"].as<std::string>();
    input = node["input"].as<GE::VInput>();
    return true;
}

}
