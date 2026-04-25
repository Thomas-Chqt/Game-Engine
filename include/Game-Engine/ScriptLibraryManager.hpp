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
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace GE
{

using ScriptLibraryListScriptNames = std::function<std::vector<std::string>()>;
using ScriptLibraryListScriptParameters = std::function<std::vector<ScriptParameterDescriptor>(const std::string&)>;
using ScriptLibraryMakeScriptInstance = std::function<std::shared_ptr<Script>(const std::string&)>;

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
