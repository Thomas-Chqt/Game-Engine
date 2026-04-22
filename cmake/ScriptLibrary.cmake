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

        GE_SCRIPT_LIBRARY_EXPORT void listScriptNames(const char*** names, unsigned long* count)
        {
            GE::ScriptRegistry::instance().listScriptNames(names, count);
        }

        GE_SCRIPT_LIBRARY_EXPORT void listScriptParameters(const char* name, const GE::ScriptParameterDescriptor** parameters, unsigned long* count)
        {
            GE::ScriptRegistry::instance().listScriptParameters(name, parameters, count);
        }

        GE_SCRIPT_LIBRARY_EXPORT GE::Script* makeScriptInstance(const char* name)
        {
            return GE::ScriptRegistry::instance().makeScriptInstance(name);
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
