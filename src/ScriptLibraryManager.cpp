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

std::vector<std::string> listScriptNames(ListScriptNamesFn fn)
{
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

std::vector<ScriptParameterDescriptor> listScriptParameters(ListScriptParametersFn fn, const std::string& scriptName)
{
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

ScriptLibraryManager::ScriptLibraryManager(const std::filesystem::path& path)
{
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
        throw std::runtime_error(std::format("unable to get symbols in lib : {}", path.string()));
}

ScriptLibraryListScriptNames ScriptLibraryManager::listScriptNamesFunction() const
{
    return ScriptLibraryListScriptNames{
        .libraryHandle = m_libraryHandle,
        .fn = m_listScriptNames
    };
}

ScriptLibraryListScriptParameters ScriptLibraryManager::listScriptParametersFunction() const
{
    return ScriptLibraryListScriptParameters{
        .libraryHandle = m_libraryHandle,
        .fn = m_listScriptParameters
    };
}

ScriptLibraryMakeScriptInstance ScriptLibraryManager::makeScriptInstanceFunction() const
{
    return ScriptLibraryMakeScriptInstance{
        .libraryHandle = m_libraryHandle,
        .fn = m_makeScriptInstance
    };
}

ScriptLibraryManager::~ScriptLibraryManager()
{
    m_listScriptNames = nullptr;
    m_listScriptParameters = nullptr;
    m_makeScriptInstance = nullptr;
    m_libraryHandle.reset();
}

std::vector<std::string> ScriptLibraryListScriptNames::operator()() const
{
    return GE::listScriptNames(fn);
}

std::vector<ScriptParameterDescriptor> ScriptLibraryListScriptParameters::operator()(const std::string& scriptName) const
{
    return GE::listScriptParameters(fn, scriptName);
}

std::shared_ptr<Script> ScriptLibraryMakeScriptInstance::operator()(const std::string& scriptName) const
{
    return GE::makeScriptInstance(libraryHandle, fn, scriptName);
}

} // namespace GE
