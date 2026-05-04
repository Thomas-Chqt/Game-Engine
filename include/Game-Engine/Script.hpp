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

#include <cassert>
#include <cstddef>
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

class GE_API Script
{
public:
    Script() = default;
    Script(const Script&) = delete;
    Script(Script&&) = delete;

    virtual void setup(Entity&, Game&) {}
    virtual void onUpdate() {}
    virtual void teardown(Entity&, Game&) {}

    virtual ~Script() = default;

    Script& operator=(const Script&) = delete;
    Script& operator=(Script&&) = delete;
};

template<typename T>
concept ScriptClass = std::derived_from<T, Script>;

class ScriptRegistry
{
public:
    static ScriptRegistry& instance()
    {
        static ScriptRegistry registry;
        return registry;
    }

    template<ScriptClass ScriptT>
    void registerScript(std::string_view name)
    {
        m_scripts.insert_or_assign(name, ScriptDescriptor{ .factory = []() -> GE::Script* { return new ScriptT; } });
    }

    template<ScriptClass ScriptT, ScriptValueType ValueT>
    void registerParameter(std::string_view scriptName, std::string_view parameterName, ValueT ScriptT::* member, ValueT defaultValue)
    {
        m_scripts.at(scriptName).parameters.insert_or_assign(parameterName, RegisteredScriptParameter{
            .typeName = ScriptValueTraits<ValueT>::name,
            .defaultValue = VScriptValue(defaultValue),
            .setParameter = [member](Script& script, const void* data) {
                static_cast<ScriptT&>(script).*member = *reinterpret_cast<const ValueT*>(data);
            }
        });
    }

    void listScriptNames(const char*** names, size_t* count) const
    {
        thread_local static std::vector<const char*> scriptNames;
        scriptNames.clear();
        scriptNames.reserve(m_scripts.size());
        for (const auto& [name, _] : m_scripts)
            scriptNames.push_back(name.data()); // ? data() safe because we know it come from a string literal
        *names = scriptNames.data();
        *count = scriptNames.size();
    }

    void listScriptParameterNames(const char* scriptName, const char*** names, size_t* count) const
    {
        const ScriptDescriptor& script = m_scripts.at(scriptName);
        thread_local static std::vector<const char*> parameterNames;
        parameterNames.clear();
        parameterNames.reserve(script.parameters.size());
        for (const auto& [name, _] : script.parameters)
            parameterNames.push_back(name.data()); // ? data() safe because we know it come from a string literal
        *names = parameterNames.data();
        *count = parameterNames.size();
    }

    const char* getScriptParameterTypeName(const char* scriptName, const char* parameterName) const
    {
        return m_scripts.at(scriptName).parameters.at(parameterName).typeName.data();
    }

    void getScriptDefaultParameterValue(const char* scriptName, const char* parameterName, void* data) const
    {
        const RegisteredScriptParameter& parameter = m_scripts.at(scriptName).parameters.at(parameterName);
        std::visit([&](const auto& defaultValue){
            using ValueT = std::remove_cvref_t<decltype(defaultValue)>;
            *reinterpret_cast<ValueT*>(data) = defaultValue;
        }, parameter.defaultValue);
    }

    GE::Script* makeScriptInstance(const char* name) const
    {
        assert(name);
        return m_scripts.at(name).factory();
    }

    void destroyScriptInstance(GE::Script* instance)
    {
        assert(instance);
        delete instance;
    }

    void setScriptParameter(const char* scriptName, const char* parameterName, GE::Script* instance, const void* data)
    {
        assert(scriptName);
        assert(parameterName);
        assert(instance);
        assert(data);
        m_scripts.at(scriptName).parameters.at(parameterName).setParameter(*instance, data);
    }

private:
    struct RegisteredScriptParameter
    {
        std::string_view typeName;
        VScriptValue defaultValue;
        std::function<void(Script&, const void*)> setParameter;
    };

    struct ScriptDescriptor
    {
        std::function<GE::Script*()> factory;
        std::map<std::string_view, RegisteredScriptParameter> parameters;
    };

    std::map<std::string_view, ScriptDescriptor> m_scripts;
};

template<typename ScriptT>
struct ScriptRegistrar
{
    ScriptRegistrar(std::string_view name)
    {
        ScriptRegistry::instance().registerScript<ScriptT>(name);
    }
};

template<typename ScriptT, ScriptValueType ValueT>
struct ScriptParameterRegistrar
{
    ScriptParameterRegistrar(std::string_view scriptName, std::string_view parameterName, ValueT ScriptT::* member, ValueT defaultValue)
    {
        ScriptRegistry::instance().registerParameter(scriptName, parameterName, member, std::move(defaultValue));
    }
};

} // namespace GE

#define GE_SCRIPT(TypeName, DisplayName)                                                       \
    using GE_ScriptSelf = TypeName;                                                            \
    static constexpr const char* ge_scriptName() { return DisplayName; }                       \
    inline static const GE::ScriptRegistrar<GE_ScriptSelf> ge_scriptRegistrar{ ge_scriptName() } \

#define GE_SCRIPT_PARAM(Type, Name, DefaultValue)                                                   \
    Type Name = DefaultValue;                                                                       \
    inline static const GE::ScriptParameterRegistrar<GE_ScriptSelf, Type> ge_paramRegistrar_##Name{ \
        ge_scriptName(), #Name, &GE_ScriptSelf::Name, DefaultValue                                  \
    }                                                                                               \

#endif // SCRIPT_HPP
