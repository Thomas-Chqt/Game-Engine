# ---------------------------------------------------
# ScriptLibrary.cmake
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# ---------------------------------------------------
# Script library helper for user project shared libraries.

function(ge_add_script_library target_name)
    set(export_source "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_ScriptSharedLibraryExports.cpp")
    file(WRITE "${export_source}"
        [=[
        #include "Game-Engine/Script.hpp"

        #if defined(_WIN32)
            #define GE_SCRIPT_LIBRARY_EXPORT __declspec(dllexport)
        #else
            #define GE_SCRIPT_LIBRARY_EXPORT __attribute__((visibility("default")))
        #endif

        extern "C"
        {

        GE_SCRIPT_LIBRARY_EXPORT void listScriptNames(const char*** names, size_t* count)
        {
            return GE::ScriptRegistry::instance().listScriptNames(names, count);
        }

        GE_SCRIPT_LIBRARY_EXPORT void listScriptParameterNames(const char* scriptName, const char*** names, size_t* count)
        {
            return GE::ScriptRegistry::instance().listScriptParameterNames(scriptName, names, count);
        }

        GE_SCRIPT_LIBRARY_EXPORT const char* getScriptParameterTypeName(const char* scriptName, const char* parameterName)
        {
            return GE::ScriptRegistry::instance().getScriptParameterTypeName(scriptName, parameterName);
        }

        GE_SCRIPT_LIBRARY_EXPORT void getScriptDefaultParameterValue(const char* scriptName, const char* parameterName, void* data)
        {
            return GE::ScriptRegistry::instance().getScriptDefaultParameterValue(scriptName, parameterName, data);
        }

        GE_SCRIPT_LIBRARY_EXPORT GE::Script* makeScriptInstance(const char* name)
        {
            return GE::ScriptRegistry::instance().makeScriptInstance(name);
        }

        GE_SCRIPT_LIBRARY_EXPORT void destroyScriptInstance(GE::Script* instance)
        {
            return GE::ScriptRegistry::instance().destroyScriptInstance(instance);
        }

        GE_SCRIPT_LIBRARY_EXPORT void setScriptParameter(const char* scriptName, const char* parameterName, GE::Script* instance, const void* data)
        {
            return GE::ScriptRegistry::instance().setScriptParameter(scriptName, parameterName, instance, data);
        }

        }
        ]=]
    )

    add_library(${target_name} SHARED ${ARGN})
    target_sources(${target_name} PRIVATE "${export_source}")
    target_compile_features(${target_name} PRIVATE cxx_std_23)

    if(APPLE)
        target_link_options(${target_name} PRIVATE "-undefined" "dynamic_lookup")
    endif()
endfunction()
