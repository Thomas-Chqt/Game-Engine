/*
 * ---------------------------------------------------
 * main.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/Application.hpp"

#include <imgui.h>

#include <memory>

#if (defined(__GNUC__) || defined(__clang__))
    #define GE_EXPORT __attribute__((used, visibility("default")))
#elif defined(_MSC_VER)
    #define GE_EXPORT __declspec(dllexport)
#else
    #error "unknown compiler"
#endif

extern "C"
{
    GE_EXPORT ImGuiContext* GetCurrentContext() { return ImGui::GetCurrentContext(); }
    GE_EXPORT ImGuiIO* GetIO() { return &ImGui::GetIO(); }
    GE_EXPORT ImGuiPlatformIO* GetPlatformIO() { return &ImGui::GetPlatformIO(); }
    GE_EXPORT ImGuiViewport* GetMainViewport() { return ImGui::GetMainViewport(); }
    GE_EXPORT bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx) { return ImGui::DebugCheckVersionAndDataLayout(version_str, sz_io, sz_style, sz_vec2, sz_vec4, sz_drawvert, sz_drawidx); }
    GE_EXPORT void* MemAlloc(size_t size) { return ImGui::MemAlloc(size); }
    GE_EXPORT void MemFree(void* ptr) { return ImGui::MemFree(ptr); }
    GE_EXPORT void DestroyPlatformWindows() { return ImGui::DestroyPlatformWindows(); }
}

extern std::unique_ptr<GE::Application> createApplication(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    createApplication(argc, argv)->run();
}
