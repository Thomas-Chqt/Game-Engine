/*
 * ---------------------------------------------------
 * ScriptLibraryManager.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

#ifndef SCRIPT_LIBRARY_MANAGER_HPP
#define SCRIPT_LIBRARY_MANAGER_HPP

#include "Game-Engine/Export.hpp"
#include "Game-Engine/Script.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace GE
{

struct GE_API ScriptLibraryListScriptNames
{
    std::shared_ptr<void> libraryHandle;
    ListScriptNamesFn fn = nullptr;

    [[nodiscard]] std::vector<std::string> operator()() const;
    [[nodiscard]] inline explicit operator bool() const { return fn != nullptr; }
};

struct GE_API ScriptLibraryListScriptParameters
{
    std::shared_ptr<void> libraryHandle;
    ListScriptParametersFn fn = nullptr;

    [[nodiscard]] std::vector<ScriptParameterDescriptor> operator()(const std::string& scriptName) const;
    [[nodiscard]] inline explicit operator bool() const { return fn != nullptr; }
};

struct GE_API ScriptLibraryMakeScriptInstance
{
    std::shared_ptr<void> libraryHandle;
    MakeScriptInstanceFn fn = nullptr;

    [[nodiscard]] std::shared_ptr<Script> operator()(const std::string& scriptName) const;
    [[nodiscard]] inline explicit operator bool() const { return fn != nullptr; }
};

class GE_API ScriptLibraryManager
{
public:
    ScriptLibraryManager(const std::filesystem::path& path);
    ScriptLibraryManager(const ScriptLibraryManager&) = delete;
    ScriptLibraryManager(ScriptLibraryManager&&) = default;

    [[nodiscard]] ScriptLibraryListScriptNames listScriptNamesFunction() const;
    [[nodiscard]] ScriptLibraryListScriptParameters listScriptParametersFunction() const;
    [[nodiscard]] ScriptLibraryMakeScriptInstance makeScriptInstanceFunction() const;

    ~ScriptLibraryManager();

    ScriptLibraryManager& operator=(const ScriptLibraryManager&) = delete;
    ScriptLibraryManager& operator=(ScriptLibraryManager&&) = default;

private:
    std::shared_ptr<void> m_libraryHandle;
    ListScriptNamesFn m_listScriptNames = nullptr;
    ListScriptParametersFn m_listScriptParameters = nullptr;
    MakeScriptInstanceFn m_makeScriptInstance = nullptr;
};

} // namespace GE

#endif // SCRIPT_LIBRARY_MANAGER_HPP
