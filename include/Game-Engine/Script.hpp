/*
 * ---------------------------------------------------
 * Script.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Export.hpp"
#include "Game-Engine/TypeList.hpp"

#include <glm/glm.hpp>

#include <concepts>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <string_view>

namespace GE
{

class Script;
class Game;
template<ECSWorldLike ECSWorldT>
struct basic_entity;
using Entity = basic_entity<ECSWorld>;

using ScriptValueTypes = TypeList<bool, int64_t, float, glm::vec2, glm::vec3, std::string>;
using VScriptValue = ScriptValueTypes::into<std::variant>;

template<typename T>
concept ScriptValueType = IsTypeInList<std::remove_cvref_t<T>, ScriptValueTypes>;

template<typename T>
struct ScriptValueTraits;

template<> struct ScriptValueTraits<bool>        { static constexpr std::string_view name = "bool"; };
template<> struct ScriptValueTraits<int64_t>     { static constexpr std::string_view name = "int"; };
template<> struct ScriptValueTraits<float>       { static constexpr std::string_view name = "float"; };
template<> struct ScriptValueTraits<glm::vec2>   { static constexpr std::string_view name = "vec2"; };
template<> struct ScriptValueTraits<glm::vec3>   { static constexpr std::string_view name = "vec3"; };
template<> struct ScriptValueTraits<std::string> { static constexpr std::string_view name = "string"; };

struct ScriptParameterDescriptor
{
    std::string name;
    VScriptValue defaultValue;
    std::function<VScriptValue(const Script&)> get;
    std::function<void(Script&, const VScriptValue&)> set;
};

class GE_API Script
{
public:
    Script() = default;
    Script(const Script&) = delete;
    Script(Script&&) = delete;

    virtual void setup(Entity&, Game&) {}
    virtual void teardown(Entity&, Game&) {}
    virtual void onUpdate() {}

    virtual ~Script() = default;

    Script& operator=(const Script&) = delete;
    Script& operator=(Script&&) = delete;
};

template<typename T>
concept ScriptClass = std::derived_from<T, Script>;

class GE_API ScriptRegistry
{
public:
    static ScriptRegistry& instance()
    {
        static ScriptRegistry registry;
        return registry;
    }

    void registerScript(std::string name, std::function<Script*()> makeScriptInstance)
    {
        m_factories.insert_or_assign(std::move(name), std::move(makeScriptInstance));
    }

    template<ScriptClass ScriptT, ScriptValueType ValueT>
    void registerParameter(const std::string& scriptName, std::string parameterName, ValueT ScriptT::* member, ValueT defaultValue)
    {
        m_parameters[scriptName].push_back(ScriptParameterDescriptor{
            .name = std::move(parameterName),
            .defaultValue = VScriptValue(defaultValue),
            .get = [member](const Script& script) -> VScriptValue {
                return static_cast<const ScriptT&>(script).*member;
            },
            .set = [member](Script& script, const VScriptValue& value) {
                static_cast<ScriptT&>(script).*member = std::get<ValueT>(value);
            }
        });
    }

    void listScriptNames(const char*** names, unsigned long* count) const
    {
        thread_local static std::vector<const char*> scriptNames;
        scriptNames.clear();
        scriptNames.reserve(m_factories.size());
        for (const auto& [name, _] : m_factories)
            scriptNames.push_back(name.c_str());
        *names = scriptNames.data();
        *count = static_cast<unsigned long>(scriptNames.size());
    }

    void listScriptParameters(const char* name, const ScriptParameterDescriptor** parameters, unsigned long* count) const
    {
        static const std::vector<ScriptParameterDescriptor> empty;
        auto it = m_parameters.find(name);
        const std::vector<ScriptParameterDescriptor>& descriptors = it == m_parameters.end() ? empty : it->second;
        *parameters = descriptors.data();
        *count = static_cast<unsigned long>(descriptors.size());
    }

    GE::Script* makeScriptInstance(const char* name) const
    {
        return m_factories.at(name)();
    }

private:
    std::map<std::string, std::function<Script*()>> m_factories;
    std::map<std::string, std::vector<ScriptParameterDescriptor>> m_parameters;
};

struct GE_API ScriptRegistrar
{
    ScriptRegistrar(std::string name, std::function<Script*()> makeScriptInstance)
    {
        ScriptRegistry::instance().registerScript(std::move(name), std::move(makeScriptInstance));
    }
};

template<typename ScriptT, ScriptValueType ValueT>
struct ScriptParameterRegistrar
{
    ScriptParameterRegistrar(const std::string& scriptName, std::string parameterName, ValueT ScriptT::* member, ValueT defaultValue)
    {
        ScriptRegistry::instance().registerParameter(scriptName, std::move(parameterName), member, defaultValue);
    }
};

} // namespace GE

#define GE_SCRIPT(TypeName, DisplayName)                                                       \
    using GE_ScriptSelf = TypeName;                                                            \
    static constexpr const char* ge_scriptName() { return DisplayName; }                       \
    inline static const GE::ScriptRegistrar ge_scriptRegistrar{                                \
        ge_scriptName(),                                                                       \
        []<typename GE_ScriptType = TypeName>() -> GE::Script* { return new GE_ScriptType(); } \
    }                                                                                          \

#define GE_SCRIPT_PARAM(Type, Name, DefaultValue)                                                   \
    Type Name = DefaultValue;                                                                       \
    inline static const GE::ScriptParameterRegistrar<GE_ScriptSelf, Type> ge_paramRegistrar_##Name{ \
        ge_scriptName(), #Name, &GE_ScriptSelf::Name, DefaultValue                                  \
    }                                                                                               \

#endif // SCRIPT_HPP
