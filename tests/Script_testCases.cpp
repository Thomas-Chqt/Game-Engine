#include <Game-Engine/Script.hpp>
#include <Game-Engine/ScriptLibrary.hpp>

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

bool containsName(const std::vector<std::string>& names, const std::string& name)
{
    return std::ranges::find(names, name) != names.end();
}

}

TEST(ScriptLibraryTest, loadsGeneratedScriptLibraryExports)
{
    EXPECT_NO_THROW({
        GE::ScriptLibrary scriptLibrary(GE_TEST_SCRIPT_LIB);

        std::vector<std::string> names = scriptLibrary.listScriptNames();

        ASSERT_EQ(names.size(), 1u);
        EXPECT_EQ(names[0], std::string("TestScript"));

        std::vector<std::string> parameters = scriptLibrary.listScriptParameterNames("TestScript");

        ASSERT_EQ(parameters.size(), 3u);
        EXPECT_TRUE(containsName(parameters, "speed"));
        EXPECT_TRUE(containsName(parameters, "enabled"));
        EXPECT_TRUE(containsName(parameters, "label"));
        EXPECT_EQ(scriptLibrary.getScriptParameterTypeName("TestScript", "speed"), "float");
        EXPECT_EQ(scriptLibrary.getScriptParameterTypeName("TestScript", "enabled"), "bool");
        EXPECT_EQ(scriptLibrary.getScriptParameterTypeName("TestScript", "label"), "string");
        EXPECT_EQ(std::get<float>(scriptLibrary.getScriptDefaultParameterValue("TestScript", "speed")), 2.5f);
        EXPECT_EQ(std::get<bool>(scriptLibrary.getScriptDefaultParameterValue("TestScript", "enabled")), true);
        EXPECT_EQ(std::get<std::string>(scriptLibrary.getScriptDefaultParameterValue("TestScript", "label")), "default");

        std::shared_ptr<GE::Script> script = scriptLibrary.makeScriptInstance("TestScript");
        ASSERT_NE(script, nullptr);
        scriptLibrary.setScriptParameter("TestScript", "speed", *script, 7.0f);
        scriptLibrary.setScriptParameter("TestScript", "enabled", *script, false);
        scriptLibrary.setScriptParameter("TestScript", "label", *script, std::string("changed"));
    });
}

TEST(ScriptLibraryTest, keepsLibraryAliveForScriptInstanceLifetime)
{
    std::shared_ptr<GE::Script> script;
    {
        GE::ScriptLibrary scriptLibrary(GE_TEST_SCRIPT_LIB);
        std::vector<std::string> parameters = scriptLibrary.listScriptParameterNames("TestScript");
        ASSERT_EQ(parameters.size(), 3u);
        EXPECT_TRUE(containsName(parameters, "speed"));
        EXPECT_EQ(std::get<float>(scriptLibrary.getScriptDefaultParameterValue("TestScript", "speed")), 2.5f);

        script = scriptLibrary.makeScriptInstance("TestScript");
        ASSERT_NE(script, nullptr);
        scriptLibrary.setScriptParameter("TestScript", "speed", *script, 11.0f);
    }

    script.reset();
}
