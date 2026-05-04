/*
 * ---------------------------------------------------
 * ScriptLibrary.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

#ifndef SCRIPT_LIBRARY_HPP
#define SCRIPT_LIBRARY_HPP

#include "Game-Engine/Export.hpp"
#include "Game-Engine/Script.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace GE
{


class GE_API ScriptLibrary
{
public:

    ScriptLibrary(const std::filesystem::path& path);
    ScriptLibrary(const ScriptLibrary&) = delete;
    ScriptLibrary(ScriptLibrary&&) = default;

    std::vector<std::string> listScriptNames() const;
    std::vector<std::string> listScriptParameterNames(const std::string& scriptName) const;
    std::string getScriptParameterTypeName(const std::string& scriptName, const std::string& parameterName) const;
    VScriptValue getScriptDefaultParameterValue(const std::string& scriptName, const std::string& parameterName) const;
    void setScriptParameter(const std::string& scriptName, const std::string& parameterName, Script& script, const VScriptValue& value) const;
    std::shared_ptr<Script> makeScriptInstance(const std::string& scriptName) const;

    ~ScriptLibrary() = default;

private:
    using MakeScriptInstanceSym = Script* (*)(const char*);
    using DestroyScriptInstanceSym = void (*)(Script*);
    using ListScriptNamesSym = void (*)(const char***, size_t*);
    using ListScriptParameterNamesSym = void (*)(const char*, const char***, size_t*);
    using GetScriptParameterTypeNameSym = const char* (*)(const char*, const char*);
    using GetScriptDefaultParameterValueSym = void (*)(const char*, const char*, void*);
    using SetScriptParameterSym = void (*)(const char*, const char*, Script*, const void*);

    std::shared_ptr<void> m_libraryHandle;

    ListScriptNamesSym m_listScriptNames = nullptr;
    ListScriptParameterNamesSym m_listScriptParameterNames = nullptr;
    GetScriptParameterTypeNameSym m_getScriptParameterTypeName = nullptr;
    GetScriptDefaultParameterValueSym m_getScriptDefaultParameterValue = nullptr;
    SetScriptParameterSym m_setScriptParameter = nullptr;
    MakeScriptInstanceSym m_makeScriptInstance = nullptr;
    DestroyScriptInstanceSym m_destroyScriptInstance = nullptr;

public:
    ScriptLibrary& operator=(const ScriptLibrary&) = delete;
    ScriptLibrary& operator=(ScriptLibrary&&) = default;
};

} // namespace GE

#endif // SCRIPT_LIBRARY_HPP
