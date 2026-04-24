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

struct GE_API ScriptLibraryFunctions
{
    ScriptLibraryListScriptNames listScriptNames;
    ScriptLibraryListScriptParameters listScriptParameters;
    ScriptLibraryMakeScriptInstance makeScriptInstance;

    [[nodiscard]] inline explicit operator bool() const
    {
        return static_cast<bool>(listScriptNames) && static_cast<bool>(listScriptParameters) && static_cast<bool>(makeScriptInstance);
    }
};

class GE_API ScriptLibraryManager
{
public:
    ScriptLibraryManager() = default;
    ScriptLibraryManager(const ScriptLibraryManager&) = delete;
    ScriptLibraryManager(ScriptLibraryManager&&) = default;

    void load(const std::filesystem::path& path);
    void unload();
    [[nodiscard]] bool isLoaded() const;

    [[nodiscard]] std::vector<std::string> listScriptNames() const;
    [[nodiscard]] std::vector<ScriptParameterDescriptor> listScriptParameters(const std::string& scriptName) const;
    [[nodiscard]] std::shared_ptr<Script> makeScriptInstance(const std::string& scriptName) const;
    [[nodiscard]] ScriptLibraryFunctions functions() const;
    [[nodiscard]] static ScriptLibraryFunctions loadFunctions(const std::filesystem::path& path);

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
