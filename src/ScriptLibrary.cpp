/*
 * ---------------------------------------------------
 * ScriptLibrary.cpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/ScriptLibrary.hpp"

#include <dlLoad/dlLoad.h>

#include <format>
#include <stdexcept>

namespace GE
{

ScriptLibrary::ScriptLibrary(const std::filesystem::path& path)
{
    DlHandle handle = dlLoad(path.string().c_str(), DL_NOW | DL_LOCAL);
    if (handle == nullptr)
        throw std::runtime_error(std::format("unable to load shared lib : {}", path.string()));

    m_libraryHandle = std::shared_ptr<void>(handle, [](void* ptr) {
        if (ptr != nullptr)
            dlFree(ptr);
    });
    m_listScriptNames = reinterpret_cast<ListScriptNamesSym>(getSym(handle, "listScriptNames"));
    m_listScriptParameterNames = reinterpret_cast<ListScriptParameterNamesSym>(getSym(handle, "listScriptParameterNames"));
    m_getScriptParameterTypeName = reinterpret_cast<GetScriptParameterTypeNameSym>(getSym(handle, "getScriptParameterTypeName"));
    m_getScriptDefaultParameterValue = reinterpret_cast<GetScriptDefaultParameterValueSym>(getSym(handle, "getScriptDefaultParameterValue"));
    m_setScriptParameter = reinterpret_cast<SetScriptParameterSym>(getSym(handle, "setScriptParameter"));
    m_makeScriptInstance = reinterpret_cast<MakeScriptInstanceSym>(getSym(handle, "makeScriptInstance"));
    m_destroyScriptInstance = reinterpret_cast<DestroyScriptInstanceSym>(getSym(handle, "destroyScriptInstance"));

    if (m_listScriptNames == nullptr
        || m_listScriptParameterNames == nullptr
        || m_getScriptParameterTypeName == nullptr
        || m_getScriptDefaultParameterValue == nullptr
        || m_setScriptParameter == nullptr
        || m_makeScriptInstance == nullptr
        || m_destroyScriptInstance == nullptr)
    {
        throw std::runtime_error(std::format("unable to get symbols in lib : {}", path.string()));
    }
}

std::vector<std::string> ScriptLibrary::listScriptNames() const
{
    const char** names = nullptr;
    size_t count = 0;
    m_listScriptNames(&names, &count);

    std::vector<std::string> output;
    output.reserve(count);
    for (size_t i = 0; i < count; i++)
        output.emplace_back(names[i]);
    return output;
}

std::vector<std::string> ScriptLibrary::listScriptParameterNames(const std::string& scriptName) const
{
    const char** parameterNames = nullptr;
    size_t count = 0;
    m_listScriptParameterNames(scriptName.c_str(), &parameterNames, &count);

    std::vector<std::string> output;
    output.reserve(count);
    for (size_t i = 0; i < count; i++)
        output.emplace_back(parameterNames[i]);
    return output;
}

std::string ScriptLibrary::getScriptParameterTypeName(const std::string& scriptName, const std::string& parameterName) const
{
    return m_getScriptParameterTypeName(scriptName.c_str(), parameterName.c_str());
}

VScriptValue ScriptLibrary::getScriptDefaultParameterValue(const std::string& scriptName, const std::string& parameterName) const
{
    const char* typeName = m_getScriptParameterTypeName(scriptName.c_str(), parameterName.c_str());
    VScriptValue defaultValue = false;
    const bool matchedDefaultType = anyOfType<ScriptValueTypes>([&]<typename T>() {
        if (ScriptValueTraits<T>::name != typeName)
            return false;

        T value{};
        m_getScriptDefaultParameterValue(scriptName.c_str(), parameterName.c_str(), &value);
        defaultValue = VScriptValue(value);
        return true;
    });
    assert(matchedDefaultType);
    return defaultValue;
}

void ScriptLibrary::setScriptParameter(const std::string& scriptName, const std::string& parameterName, Script& script, const VScriptValue& value) const
{
    const char* typeName = m_getScriptParameterTypeName(scriptName.c_str(), parameterName.c_str());
    const bool matchedSetType = anyOfType<ScriptValueTypes>([&]<typename T>() {
        if (ScriptValueTraits<T>::name != typeName)
            return false;

        T typedValue = std::get<T>(value);
        m_setScriptParameter(scriptName.c_str(), parameterName.c_str(), &script, &typedValue);
        return true;
    });
    assert(matchedSetType);
}

std::shared_ptr<Script> ScriptLibrary::makeScriptInstance(const std::string& scriptName) const
{
    Script* script = m_makeScriptInstance(scriptName.c_str());
    if (script == nullptr)
        return {};

    return std::shared_ptr<Script>(script, [libraryHandle = m_libraryHandle, destroyScriptInstanceFn = m_destroyScriptInstance](Script* ptr) {
        (void)libraryHandle;
        destroyScriptInstanceFn(ptr);
    });
}

} // namespace GE
