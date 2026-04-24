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
#include <utility>

namespace GE
{

namespace
{

std::vector<std::string> listScriptNames(std::shared_ptr<void> libraryHandle, ListScriptNamesFn fn)
{
    (void)libraryHandle;
    if (fn == nullptr)
        return {};

    const char** names = nullptr;
    unsigned long count = 0;
    fn(&names, &count);

    std::vector<std::string> output;
    output.reserve(count);
    for (unsigned long i = 0; i < count; i++)
        if (names[i] != nullptr)
            output.emplace_back(names[i]);
    return output;
}

std::vector<ScriptParameterDescriptor> listScriptParameters(std::shared_ptr<void> libraryHandle, ListScriptParametersFn fn, const std::string& scriptName)
{
    (void)libraryHandle;
    if (fn == nullptr)
        return {};

    const ScriptParameterDescriptor* parameters = nullptr;
    unsigned long count = 0;
    fn(scriptName.c_str(), &parameters, &count);

    std::vector<ScriptParameterDescriptor> output;
    output.reserve(count);
    for (unsigned long i = 0; i < count; i++)
        output.emplace_back(parameters[i]);
    return output;
}

std::shared_ptr<Script> makeScriptInstance(std::shared_ptr<void> libraryHandle, MakeScriptInstanceFn fn, const std::string& scriptName)
{
    if (fn == nullptr)
        return {};

    Script* script = fn(scriptName.c_str());
    if (script == nullptr)
        return {};

    return std::shared_ptr<Script>(script, [libraryHandle = std::move(libraryHandle)](Script* ptr) {
        delete ptr;
    });
}

} // namespace

void ScriptLibraryManager::load(const std::filesystem::path& path)
{
    unload();

    DlHandle handle = dlLoad(path.string().c_str());
    if (handle == nullptr)
        throw std::runtime_error(std::format("unable to load shared lib : {}", path.string()));

    m_libraryHandle = std::shared_ptr<void>(handle, [](void* ptr){
        if (ptr != nullptr)
            dlFree(ptr);
    });
    m_listScriptNames = reinterpret_cast<ListScriptNamesFn>(getSym(handle, "listScriptNames"));
    m_listScriptParameters = reinterpret_cast<ListScriptParametersFn>(getSym(handle, "listScriptParameters"));
    m_makeScriptInstance = reinterpret_cast<MakeScriptInstanceFn>(getSym(handle, "makeScriptInstance"));

    if (m_listScriptNames == nullptr || m_listScriptParameters == nullptr || m_makeScriptInstance == nullptr)
    {
        unload();
        throw std::runtime_error(std::format("unable to get symbols in lib : {}", path.string()));
    }
}

void ScriptLibraryManager::unload()
{
    m_listScriptNames = nullptr;
    m_listScriptParameters = nullptr;
    m_makeScriptInstance = nullptr;
    m_libraryHandle.reset();
}

bool ScriptLibraryManager::isLoaded() const
{
    return m_libraryHandle != nullptr;
}

std::vector<std::string> ScriptLibraryManager::listScriptNames() const
{
    return GE::listScriptNames(m_libraryHandle, m_listScriptNames);
}

std::vector<ScriptParameterDescriptor> ScriptLibraryManager::listScriptParameters(const std::string& scriptName) const
{
    return GE::listScriptParameters(m_libraryHandle, m_listScriptParameters, scriptName);
}

std::shared_ptr<Script> ScriptLibraryManager::makeScriptInstance(const std::string& scriptName) const
{
    return GE::makeScriptInstance(m_libraryHandle, m_makeScriptInstance, scriptName);
}

ScriptLibraryFunctions ScriptLibraryManager::functions() const
{
    return ScriptLibraryFunctions{
        .listScriptNames = ScriptLibraryListScriptNames{
            .libraryHandle = m_libraryHandle,
            .fn = m_listScriptNames
        },
        .listScriptParameters = ScriptLibraryListScriptParameters{
            .libraryHandle = m_libraryHandle,
            .fn = m_listScriptParameters
        },
        .makeScriptInstance = ScriptLibraryMakeScriptInstance{
            .libraryHandle = m_libraryHandle,
            .fn = m_makeScriptInstance
        },
    };
}

ScriptLibraryFunctions ScriptLibraryManager::loadFunctions(const std::filesystem::path& path)
{
    ScriptLibraryManager manager;
    manager.load(path);
    return manager.functions();
}

ScriptLibraryManager::~ScriptLibraryManager()
{
    unload();
}

std::vector<std::string> ScriptLibraryListScriptNames::operator()() const
{
    return GE::listScriptNames(libraryHandle, fn);
}

std::vector<ScriptParameterDescriptor> ScriptLibraryListScriptParameters::operator()(const std::string& scriptName) const
{
    return GE::listScriptParameters(libraryHandle, fn, scriptName);
}

std::shared_ptr<Script> ScriptLibraryMakeScriptInstance::operator()(const std::string& scriptName) const
{
    return GE::makeScriptInstance(libraryHandle, fn, scriptName);
}

} // namespace GE
