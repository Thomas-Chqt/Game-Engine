#include <Game-Engine/Script.hpp>
#include <Game-Engine/ScriptLibraryManager.hpp>

#include <dlLoad/dlLoad.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

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
    EXPECT_NO_THROW({
        GE::ScriptLibraryManager manager(GE_TEST_SCRIPT_LIB);
        GE::ListScriptNamesFn listScriptNames = manager.listScriptNamesFunction();
        GE::ListScriptParametersFn listScriptParameters = manager.listScriptParametersFunction();
        GE::MakeScriptInstanceFn makeScriptInstance = manager.makeScriptInstanceFunction();

        std::vector<std::string> names = listScriptNames();

        ASSERT_EQ(names.size(), 1u);
        EXPECT_EQ(names[0], std::string("TestScript"));

        std::vector<GE::ScriptParameterDescriptor> parameters = listScriptParameters("TestScript");

        ASSERT_EQ(parameters.size(), 3u);

        const GE::ScriptParameterDescriptor* speed = findParameter(parameters.data(), parameters.size(), "speed");
        const GE::ScriptParameterDescriptor* enabled = findParameter(parameters.data(), parameters.size(), "enabled");
        const GE::ScriptParameterDescriptor* label = findParameter(parameters.data(), parameters.size(), "label");

        ASSERT_NE(speed, nullptr);
        ASSERT_NE(enabled, nullptr);
        ASSERT_NE(label, nullptr);

        EXPECT_EQ(std::get<float>(speed->defaultValue), 2.5f);
        EXPECT_EQ(std::get<bool>(enabled->defaultValue), true);
        EXPECT_EQ(std::get<std::string>(label->defaultValue), "default");

        std::shared_ptr<GE::Script> script = makeScriptInstance("TestScript");
        ASSERT_NE(script, nullptr);

        speed->set(*script, 7.0f);
        enabled->set(*script, false);
        label->set(*script, std::string("changed"));

        EXPECT_EQ(std::get<float>(speed->get(*script)), 7.0f);
        EXPECT_EQ(std::get<bool>(enabled->get(*script)), false);
        EXPECT_EQ(std::get<std::string>(label->get(*script)), "changed");
    });
}

TEST(ScriptLibraryManagerTest, keepsLibraryAliveForScriptInstanceLifetime)
{
    GE::ListScriptParametersFn listScriptParameters;
    GE::MakeScriptInstanceFn makeScriptInstance;
    {
        GE::ScriptLibraryManager manager(GE_TEST_SCRIPT_LIB);
        listScriptParameters = manager.listScriptParametersFunction();
        makeScriptInstance = manager.makeScriptInstanceFunction();
    }

    std::vector<GE::ScriptParameterDescriptor> parameters = listScriptParameters("TestScript");
    ASSERT_EQ(parameters.size(), 3u);
    const GE::ScriptParameterDescriptor* speed = findParameter(parameters.data(), parameters.size(), "speed");
    ASSERT_NE(speed, nullptr);

    std::shared_ptr<GE::Script> script = makeScriptInstance("TestScript");
    ASSERT_NE(script, nullptr);

    speed->set(*script, 11.0f);
    EXPECT_EQ(std::get<float>(speed->get(*script)), 11.0f);

    script.reset();
}
