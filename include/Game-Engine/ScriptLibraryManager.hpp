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

    ~ScriptLibraryManager();

    ScriptLibraryManager& operator=(const ScriptLibraryManager&) = delete;
    ScriptLibraryManager& operator=(ScriptLibraryManager&&) = default;

private:
    struct LibraryHandle;
    std::shared_ptr<LibraryHandle> m_libraryHandle;
    ListScriptNamesFn m_listScriptNames = nullptr;
    ListScriptParametersFn m_listScriptParameters = nullptr;
    MakeScriptInstanceFn m_makeScriptInstance = nullptr;
};

} // namespace GE

#endif // SCRIPT_LIBRARY_MANAGER_HPP
