#include "convert_components.hpp"

#include "convert_vector.hpp" // IWYU pragma: keep

namespace
{

template<typename T>
struct ECSComponentYamlTraits;

template<> struct ECSComponentYamlTraits<GE::NameComponent>      { static constexpr std::string_view name = "NameComponent";      };
template<> struct ECSComponentYamlTraits<GE::ParentComponent>    { static constexpr std::string_view name = "ParentComponent";    };
template<> struct ECSComponentYamlTraits<GE::ChildrenComponent>  { static constexpr std::string_view name = "ChildrenComponent";  };
template<> struct ECSComponentYamlTraits<GE::TransformComponent> { static constexpr std::string_view name = "TransformComponent"; };
template<> struct ECSComponentYamlTraits<GE::CameraComponent>    { static constexpr std::string_view name = "CameraComponent";    };
template<> struct ECSComponentYamlTraits<GE::LightComponent>     { static constexpr std::string_view name = "LightComponent";     };
template<> struct ECSComponentYamlTraits<GE::MeshComponent>      { static constexpr std::string_view name = "MeshComponent";      };
template<> struct ECSComponentYamlTraits<GE::ScriptComponent>    { static constexpr std::string_view name = "ScriptComponent";    };

}

namespace YAML
{

template<>
struct convert<GE::NameComponent>
{
    static Node encode(const GE::NameComponent& rhs)
    {
        Node node;
        node["name"] = rhs.name;
        return node;
    }

    static bool decode(const Node& node, GE::NameComponent& rhs)
    {
        if (!node.IsMap() || !node["name"])
            return false;

        rhs.name = node["name"].as<std::string>();
        return true;
    }
};

template<>
struct convert<GE::ParentComponent>
{
    static Node encode(const GE::ParentComponent& rhs)
    {
        Node node;
        node["parent"] = rhs.parent;
        node["nextChild"] = rhs.nextChild;
        return node;
    }

    static bool decode(const Node& node, GE::ParentComponent& rhs)
    {
        if (!node.IsMap())
            return false;

        rhs.parent = node["parent"] ? node["parent"].as<GE::EntityID>() : GE::INVALID_ENTITY_ID;
        rhs.nextChild = node["nextChild"] ? node["nextChild"].as<GE::EntityID>() : GE::INVALID_ENTITY_ID;
        return true;
    }
};

template<>
struct convert<GE::ChildrenComponent>
{
    static Node encode(const GE::ChildrenComponent& rhs)
    {
        Node node;
        node["firstChild"] = rhs.firstChild;
        return node;
    }

    static bool decode(const Node& node, GE::ChildrenComponent& rhs)
    {
        if (!node.IsMap())
            return false;

        rhs.firstChild = node["firstChild"] ? node["firstChild"].as<GE::EntityID>() : GE::INVALID_ENTITY_ID;
        return true;
    }
};

template<>
struct convert<GE::TransformComponent>
{
    static Node encode(const GE::TransformComponent& rhs)
    {
        Node node;
        node["position"] = rhs.position;
        node["rotation"] = rhs.rotation;
        node["scale"] = rhs.scale;
        return node;
    }

    static bool decode(const Node& node, GE::TransformComponent& rhs)
    {
        if (!node.IsMap())
            return false;

        rhs.position = node["position"] ? node["position"].as<glm::vec3>() : glm::vec3{0.0f, 0.0f, 0.0f};
        if (const Node rotationNode = node["rotation"])
        {
            rhs.rotation = rotationNode["w"]
                ? rotationNode.as<glm::quat>()
                : glm::normalize(glm::quat(rotationNode.as<glm::vec3>()));
        }
        else
        {
            rhs.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }
        rhs.scale = node["scale"] ? node["scale"].as<glm::vec3>() : glm::vec3{1.0f, 1.0f, 1.0f};
        return true;
    }
};

template<>
struct convert<GE::CameraComponent>
{
    static Node encode(const GE::CameraComponent& rhs)
    {
        Node node;
        node["fov"] = rhs.fov;
        node["zFar"] = rhs.zFar;
        node["zNear"] = rhs.zNear;
        return node;
    }

    static bool decode(const Node& node, GE::CameraComponent& rhs)
    {
        if (!node.IsMap())
            return false;

        rhs.fov = node["fov"] ? node["fov"].as<float>() : glm::radians(60.0f);
        rhs.zFar = node["zFar"] ? node["zFar"].as<float>() : 1000.0f;
        rhs.zNear = node["zNear"] ? node["zNear"].as<float>() : 0.1f;
        return true;
    }
};

template<>
struct convert<GE::LightComponent::Type>
{
    static Node encode(const GE::LightComponent::Type& rhs)
    {
        switch (rhs)
        {
            case GE::LightComponent::Type::directional:
                return Node("directional");
            case GE::LightComponent::Type::point:
                return Node("point");
        }
        return Node{};
    }

    static bool decode(const Node& node, GE::LightComponent::Type& rhs)
    {
        if (!node.IsScalar())
            return false;

        const auto value = node.as<std::string>();
        if (value == "directional")
            rhs = GE::LightComponent::Type::directional;
        else if (value == "point")
            rhs = GE::LightComponent::Type::point;
        else
            return false;

        return true;
    }
};

template<>
struct convert<GE::LightComponent>
{
    static Node encode(const GE::LightComponent& rhs)
    {
        Node node;
        node["type"] = rhs.type;
        node["color"] = rhs.color;
        node["intentsity"] = rhs.intentsity;
        node["attenuation"] = rhs.attenuation;
        return node;
    }

    static bool decode(const Node& node, GE::LightComponent& rhs)
    {
        if (!node.IsMap())
            return false;

        rhs.type = node["type"] ? node["type"].as<GE::LightComponent::Type>() : GE::LightComponent::Type::point;
        rhs.color = node["color"] ? node["color"].as<glm::vec3>() : glm::vec3{1.0f, 1.0f, 1.0f};
        rhs.intentsity = node["intentsity"] ? node["intentsity"].as<float>() : 1.0f;
        rhs.attenuation = node["attenuation"] ? node["attenuation"].as<float>() : 0.0f;
        return true;
    }
};

template<>
struct convert<GE::MeshComponent>
{
    static Node encode(const GE::MeshComponent& rhs)
    {
        Node node;
        node["id"] = rhs.id;
        return node;
    }

    static bool decode(const Node& node, GE::MeshComponent& rhs)
    {
        if (!node.IsMap() || !node["id"])
            return false;

        rhs.id = node["id"].as<GE::AssetID>();
        return true;
    }
};

template<>
struct convert<GE::VScriptValue>
{
    static Node encode(const GE::VScriptValue& rhs)
    {
        Node node;
        std::visit([&](const auto& value) {
            using ValueT = std::remove_cvref_t<decltype(value)>;
            node["type"] = std::string(GE::ScriptValueTraits<ValueT>::name);
            node["data"] = value;
        }, rhs);
        return node;
    }

    static bool decode(const Node& node, GE::VScriptValue& rhs)
    {
        if (!node.IsMap() || !node["type"] || !node["data"])
            return false;

        const std::string type = node["type"].as<std::string>();
        const Node data = node["data"];
        return GE::anyOfType<GE::ScriptValueTypes>([&]<typename ValueT>() {
            if (type != GE::ScriptValueTraits<ValueT>::name)
                return false;
            rhs = data.as<ValueT>();
            return true;
        });
    }
};

template<>
struct convert<GE::ScriptComponent>
{
    static Node encode(const GE::ScriptComponent& rhs)
    {
        Node node;
        node["name"] = rhs.name;
        node["parameters"] = rhs.parameters;
        return node;
    }

    static bool decode(const Node& node, GE::ScriptComponent& rhs)
    {
        if (!node.IsMap() || !node["name"])
            return false;

        rhs.name = node["name"].as<std::string>();
        rhs.parameters = node["parameters"] ? node["parameters"].as<std::map<std::string, GE::VScriptValue>>() : std::map<std::string, GE::VScriptValue>();
        rhs.instance.reset();
        return true;
    }
};

template<>
struct convert<GE::ComponentVariant>
{
    static Node encode(const GE::ComponentVariant& rhs)
    {
        Node node;
        std::visit([&](const auto& component) {
            using ComponentT = std::remove_cvref_t<decltype(component)>;
            node["type"] = std::string(ECSComponentYamlTraits<ComponentT>::name);
            node["data"] = component;
        }, rhs);
        return node;
    }

    static bool decode(const Node& node, GE::ComponentVariant& rhs)
    {
        if (!node.IsMap() || !node["type"] || !node["data"])
            return false;

        const auto type = node["type"].as<std::string>();
        const Node data = node["data"];
        return GE::anyOfType<GE::ECSComponentTypes>([&]<typename ComponentT>() {
            if (type != ECSComponentYamlTraits<ComponentT>::name)
                return false;
            rhs = data.as<ComponentT>();
            return true;
        });
    }
};

Node convert<std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>>::encode(const std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>& rhs)
{
    Node node;
    auto& [id, components] = rhs;
    node["entityId"] = id;
    node["components"] = components;
    return node;
}

bool convert<std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>>::decode(const Node& node, std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>& rhs)
{
    if (!node.IsMap() || !node["entityId"] || !node["components"])
        return false;

    auto& [id, components] = rhs;
    id = node["entityId"].as<GE::EntityID>();
    components.clear();
    if (!node["components"].IsSequence())
        return false;
    for (const Node& componentNode : node["components"])
    {
        if (!componentNode.IsMap() || !componentNode["type"] || !componentNode["data"])
            return false;

        const std::string type = componentNode["type"].as<std::string>();
        if (type == "HierarchyComponent")
        {
            const Node data = componentNode["data"];
            if (!data.IsMap())
                return false;

            const GE::EntityID parent = data["parent"] ? data["parent"].as<GE::EntityID>() : GE::INVALID_ENTITY_ID;
            const GE::EntityID firstChild = data["firstChild"] ? data["firstChild"].as<GE::EntityID>() : GE::INVALID_ENTITY_ID;
            const GE::EntityID nextChild = data["nextChild"] ? data["nextChild"].as<GE::EntityID>() : GE::INVALID_ENTITY_ID;

            if (parent != GE::INVALID_ENTITY_ID || nextChild != GE::INVALID_ENTITY_ID)
                components.emplace_back(GE::ParentComponent{.parent = parent, .nextChild = nextChild});
            if (firstChild != GE::INVALID_ENTITY_ID)
                components.emplace_back(GE::ChildrenComponent{.firstChild = firstChild});
            continue;
        }

        components.emplace_back(componentNode.as<GE::ComponentVariant>());
    }
    return true;
}

}
