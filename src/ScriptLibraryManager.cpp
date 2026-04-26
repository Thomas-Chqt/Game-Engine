/*
 * ---------------------------------------------------
 * ScriptLibraryManager.cpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/ScriptLibraryManager.hpp"

#include <dlLoad/dlLoad.h>

#include <format>
#include <stdexcept>
namespace GE
{

ScriptLibraryManager::ScriptLibraryManager(const std::filesystem::path& path)
{
    DlHandle handle = dlLoad(path.string().c_str());
    if (handle == nullptr)
        throw std::runtime_error(std::format("unable to load shared lib : {}", path.string()));

    m_libraryHandle = std::shared_ptr<void>(handle, [](void* ptr){
        if (ptr != nullptr)
            dlFree(ptr);
    });
    m_listScriptNames = reinterpret_cast<ListScriptNamesSym>(getSym(handle, "listScriptNames"));
    m_listScriptParameters = reinterpret_cast<ListScriptParametersSym>(getSym(handle, "listScriptParameters"));
    m_makeScriptInstance = reinterpret_cast<MakeScriptInstanceSym>(getSym(handle, "makeScriptInstance"));

    if (m_listScriptNames == nullptr || m_listScriptParameters == nullptr || m_makeScriptInstance == nullptr)
        throw std::runtime_error(std::format("unable to get symbols in lib : {}", path.string()));
}

ListScriptNamesFn ScriptLibraryManager::listScriptNamesFunction() const
{
    return [libraryHandle = m_libraryHandle, listScriptNamesFn = m_listScriptNames]() -> std::vector<std::string> {
        (void)libraryHandle;
        if (listScriptNamesFn == nullptr)
            return {};

        const char** names = nullptr;
        unsigned long count = 0;
        listScriptNamesFn(&names, &count);

        std::vector<std::string> output;
        output.reserve(count);
        for (unsigned long i = 0; i < count; i++)
            if (names[i] != nullptr)
                output.emplace_back(names[i]);
        return output;
    };
}

ListScriptParametersFn ScriptLibraryManager::listScriptParametersFunction() const
{
    return [libraryHandle = m_libraryHandle, listScriptParametersFn = m_listScriptParameters](const std::string& scriptName) -> std::vector<ScriptParameterDescriptor> {
        (void)libraryHandle;
        if (listScriptParametersFn == nullptr)
            return {};

        const ScriptParameterDescriptor* parameters = nullptr;
        unsigned long count = 0;
        listScriptParametersFn(scriptName.c_str(), &parameters, &count);

        std::vector<ScriptParameterDescriptor> output;
        output.reserve(count);
        for (unsigned long i = 0; i < count; i++)
            output.emplace_back(parameters[i]);
        return output;
    };
}

MakeScriptInstanceFn ScriptLibraryManager::makeScriptInstanceFunction() const
{
    return [libraryHandle = m_libraryHandle, makeScriptInstanceFn = m_makeScriptInstance](const std::string& scriptName) -> std::shared_ptr<Script> {
        if (makeScriptInstanceFn == nullptr)
            return {};

        Script* script = makeScriptInstanceFn(scriptName.c_str());
        if (script == nullptr)
            return {};

        return std::shared_ptr<Script>(script, [libraryHandle](Script* ptr) {
            delete ptr;
        });
    };
}

ScriptLibraryManager::~ScriptLibraryManager()
{
    m_listScriptNames = nullptr;
    m_listScriptParameters = nullptr;
    m_makeScriptInstance = nullptr;
    m_libraryHandle.reset();
}

} // namespace GE
