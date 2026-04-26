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


using ListScriptNamesFn = std::function<std::vector<std::string>()>;
using ListScriptParametersFn = std::function<std::vector<ScriptParameterDescriptor>(const std::string&)>;
using MakeScriptInstanceFn = std::function<std::shared_ptr<Script>(const std::string&)>;

class GE_API ScriptLibraryManager
{
public:

    ScriptLibraryManager(const std::filesystem::path& path);
    ScriptLibraryManager(const ScriptLibraryManager&) = delete;
    ScriptLibraryManager(ScriptLibraryManager&&) = default;

    ListScriptNamesFn listScriptNamesFunction() const;
    ListScriptParametersFn listScriptParametersFunction() const;
    MakeScriptInstanceFn makeScriptInstanceFunction() const;

    ~ScriptLibraryManager();

    ScriptLibraryManager& operator=(const ScriptLibraryManager&) = delete;
    ScriptLibraryManager& operator=(ScriptLibraryManager&&) = default;

private:
    using MakeScriptInstanceSym = Script* (*)(const char*);
    using ListScriptNamesSym = void (*)(const char***, unsigned long*);
    using ListScriptParametersSym = void (*)(const char*, const ScriptParameterDescriptor**, unsigned long*);

    std::shared_ptr<void> m_libraryHandle;

    ListScriptNamesSym m_listScriptNames = nullptr;
    ListScriptParametersSym m_listScriptParameters = nullptr;
    MakeScriptInstanceSym m_makeScriptInstance = nullptr;
};

} // namespace GE

#endif // SCRIPT_LIBRARY_MANAGER_HPP
