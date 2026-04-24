#include <Game-Engine/Script.hpp>
#include <Game-Engine/ScriptLibraryManager.hpp>

#include <dlLoad/dlLoad.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>

static_assert(GE::ScriptValueType<bool>);
static_assert(GE::ScriptValueType<int64_t>);
static_assert(GE::ScriptValueType<float>);
static_assert(GE::ScriptValueType<glm::vec2>);
static_assert(GE::ScriptValueType<glm::vec3>);
static_assert(GE::ScriptValueType<std::string>);
static_assert(!GE::ScriptValueType<double>);

namespace
{

struct DlHandleDeleter
{
    void operator()(DlHandle handle) const
    {
        if (handle != nullptr)
            dlFree(handle);
    }
};

using UniqueDlHandle = std::unique_ptr<std::remove_pointer_t<DlHandle>, DlHandleDeleter>;

template<typename FnT>
FnT loadFunction(DlHandle handle, const char* name)
{
    void* symbol = getSym(handle, name);
    EXPECT_NE(symbol, nullptr) << name;
    return reinterpret_cast<FnT>(symbol);
}

const GE::ScriptParameterDescriptor* findParameter(const GE::ScriptParameterDescriptor* parameters, unsigned long count, const std::string& name)
{
    auto* end = parameters + count;
    auto* it = std::find_if(parameters, end, [&](const GE::ScriptParameterDescriptor& parameter) {
        return parameter.name == name;
    });
    return it == end ? nullptr : it;
}

}

TEST(ScriptLibraryTest, loadsGeneratedScriptLibraryExports)
{
    UniqueDlHandle handle(dlLoad(GE_TEST_SCRIPT_LIB));
    ASSERT_NE(handle, nullptr);

    auto listScriptNames = loadFunction<GE::ListScriptNamesFn>(handle.get(), "listScriptNames");
    auto listScriptParameters = loadFunction<GE::ListScriptParametersFn>(handle.get(), "listScriptParameters");
    auto makeScriptInstance = loadFunction<GE::MakeScriptInstanceFn>(handle.get(), "makeScriptInstance");

    const char** names = nullptr;
    unsigned long nameCount = 0;
    listScriptNames(&names, &nameCount);

    ASSERT_NE(names, nullptr);
    ASSERT_EQ(nameCount, 1u);
    EXPECT_STREQ(names[0], "TestScript");

    const GE::ScriptParameterDescriptor* parameters = nullptr;
    unsigned long parameterCount = 0;
    listScriptParameters("TestScript", &parameters, &parameterCount);

    ASSERT_NE(parameters, nullptr);
    ASSERT_EQ(parameterCount, 3u);

    const GE::ScriptParameterDescriptor* speed = findParameter(parameters, parameterCount, "speed");
    const GE::ScriptParameterDescriptor* enabled = findParameter(parameters, parameterCount, "enabled");
    const GE::ScriptParameterDescriptor* label = findParameter(parameters, parameterCount, "label");

    ASSERT_NE(speed, nullptr);
    ASSERT_NE(enabled, nullptr);
    ASSERT_NE(label, nullptr);

    EXPECT_EQ(std::get<float>(speed->defaultValue), 2.5f);
    EXPECT_EQ(std::get<bool>(enabled->defaultValue), true);
    EXPECT_EQ(std::get<std::string>(label->defaultValue), "default");

    std::unique_ptr<GE::Script> script(makeScriptInstance("TestScript"));
    ASSERT_NE(script, nullptr);

    speed->set(*script, 7.0f);
    enabled->set(*script, false);
    label->set(*script, std::string("changed"));

    EXPECT_EQ(std::get<float>(speed->get(*script)), 7.0f);
    EXPECT_EQ(std::get<bool>(enabled->get(*script)), false);
    EXPECT_EQ(std::get<std::string>(label->get(*script)), "changed");
}

TEST(ScriptLibraryManagerTest, keepsLibraryAliveForScriptInstanceLifetime)
{
    GE::ScriptLibraryManager manager;
    manager.load(GE_TEST_SCRIPT_LIB);

    std::vector<GE::ScriptParameterDescriptor> parameters = manager.listScriptParameters("TestScript");
    ASSERT_EQ(parameters.size(), 3u);
    const GE::ScriptParameterDescriptor* speed = findParameter(parameters.data(), parameters.size(), "speed");
    ASSERT_NE(speed, nullptr);

    std::shared_ptr<GE::Script> script = manager.makeScriptInstance("TestScript");
    ASSERT_NE(script, nullptr);

    manager.unload();
    EXPECT_FALSE(manager.isLoaded());

    speed->set(*script, 11.0f);
    EXPECT_EQ(std::get<float>(speed->get(*script)), 11.0f);

    script.reset();
}
