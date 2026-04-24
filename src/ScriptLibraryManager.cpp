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

struct ScriptLibraryManager::LibraryHandle
{
    explicit LibraryHandle(DlHandle handle)
        : value(handle)
    {
    }

    ~LibraryHandle()
    {
        if (value != nullptr)
            dlFree(value);
    }

    DlHandle value = nullptr;
};

void ScriptLibraryManager::load(const std::filesystem::path& path)
{
    unload();

    DlHandle handle = dlLoad(path.string().c_str());
    if (handle == nullptr)
        throw std::runtime_error(std::format("unable to load shared lib : {}", path.string()));

    m_libraryHandle = std::make_shared<LibraryHandle>(handle);
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
    if (m_listScriptNames == nullptr)
        return {};

    const char** names = nullptr;
    unsigned long count = 0;
    m_listScriptNames(&names, &count);

    std::vector<std::string> output;
    output.reserve(count);
    for (unsigned long i = 0; i < count; i++)
        if (names[i] != nullptr)
            output.emplace_back(names[i]);
    return output;
}

std::vector<ScriptParameterDescriptor> ScriptLibraryManager::listScriptParameters(const std::string& scriptName) const
{
    if (m_listScriptParameters == nullptr)
        return {};

    const ScriptParameterDescriptor* parameters = nullptr;
    unsigned long count = 0;
    m_listScriptParameters(scriptName.c_str(), &parameters, &count);

    std::vector<ScriptParameterDescriptor> output;
    output.reserve(count);
    for (unsigned long i = 0; i < count; i++)
        output.emplace_back(parameters[i]);
    return output;
}

std::shared_ptr<Script> ScriptLibraryManager::makeScriptInstance(const std::string& scriptName) const
{
    struct ScriptInstanceDeleter
    {
        std::shared_ptr<LibraryHandle> libraryHandle;

        void operator()(Script* ptr) const
        {
            delete ptr;
        }
    };

    if (m_makeScriptInstance == nullptr)
        return {};

    Script* script = m_makeScriptInstance(scriptName.c_str());
    if (script == nullptr)
        return {};

    return std::shared_ptr<Script>(script, ScriptInstanceDeleter{
        .libraryHandle = m_libraryHandle
    });
}

ScriptLibraryManager::~ScriptLibraryManager()
{
    unload();
}

} // namespace GE
