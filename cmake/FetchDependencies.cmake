# ---------------------------------------------------
# FetchDependencies.cmake
#
# Centralizes FetchContent-based dependencies for the Game-Engine project.
# Call `fetch_dependencies()` from the root `CMakeLists.txt`.
# ---------------------------------------------------

include_guard(GLOBAL)

include(FetchContent)

function(fetch_dependencies)
    set(FETCHCONTENT_QUIET OFF)
    set(ENV{GIT_LFS_SKIP_SMUDGE} 1)

    # -----------------------------
    # Graphics
    # -----------------------------
    FetchContent_Declare(Graphics
        GIT_REPOSITORY    https://github.com/Thomas-Chqt/Graphics.git
        GIT_TAG           c85bcc401cdeaa461458ac1078ca441ad200b65f
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(GFX_BUILD_METAL      ${GE_BUILD_METAL})
    set(GFX_BUILD_VULKAN     ${GE_BUILD_VULKAN})
    set(GFX_ENABLE_IMGUI     ON)
    set(GFX_ENABLE_GLFW      ON)
    set(GFX_BUILD_EXAMPLES   OFF)
    set(GFX_BUILD_TESTS      OFF)
    set(GFX_BUILD_TRACY      OFF)
    set(GFX_INSTALL          ${GE_INSTALL})
    FetchContent_MakeAvailable(Graphics)
    set_target_properties(Graphics PROPERTIES FOLDER "dependencies/Graphics")
    set_target_properties(gfxsc PROPERTIES FOLDER "dependencies/Graphics")

    # -----------------------------
    # GLFW
    # -----------------------------
    FetchContent_Declare(glfw3
        GIT_REPOSITORY    https://github.com/glfw/glfw.git
        GIT_TAG           3.4
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(GLFW_BUILD_TESTS    OFF)
    set(GLFW_BUILD_DOCS     OFF)
    set(GLFW_INSTALL        OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    if(NOT GLFW_BUILD_WAYLAND)
        set(GLFW_BUILD_WAYLAND OFF)
    endif()
    if (APPLE)
        enable_language(OBJC)
    endif()
    FetchContent_MakeAvailable(glfw3)
    if (glfw3_SOURCE_DIR)
        if (TARGET update_mappings)
            set_target_properties(glfw PROPERTIES FOLDER "dependencies/GLFW3")
            set_target_properties(update_mappings PROPERTIES FOLDER "dependencies/GLFW3")
        else()
            set_target_properties(glfw PROPERTIES FOLDER "dependencies")
        endif()
    endif()

    # -----------------------------
    # GLM
    # -----------------------------
    FetchContent_Declare(glm
        GIT_REPOSITORY    https://github.com/g-truc/glm.git
        GIT_TAG           1.0.1
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(glm)
    if (glm_SOURCE_DIR)
        set_target_properties(glm PROPERTIES FOLDER "dependencies")
    endif()

    # -----------------------------
    # ImGui
    # -----------------------------
    FetchContent_Declare(imgui
        GIT_REPOSITORY    https://github.com/Thomas-Chqt/imgui.git
        GIT_TAG           4d8f55183c2e974916daf6336ee2c4b9c8e72891
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(IM_BUILD_GLFW ON)
    FetchContent_MakeAvailable(imgui)
    target_compile_definitions(imgui PRIVATE "GLFW_INCLUDE_NONE")
    target_link_libraries(imgui PUBLIC glfw)
    if (imgui_SOURCE_DIR)
        set_target_properties(imgui PROPERTIES FOLDER "dependencies")
    endif()

    # -----------------------------
    # stb_image
    # -----------------------------
    FetchContent_Declare(stb_image
        GIT_REPOSITORY    https://github.com/Thomas-Chqt/stb_image.git
        GIT_TAG           a09b799a2ba95c14f511493db4ea59811b2fcc97
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(stb_image)
    if (stb_image_SOURCE_DIR)
        set_target_properties(stb_image PROPERTIES FOLDER "dependencies")
    endif()

    # -----------------------------
    # dlLoad
    # -----------------------------
    FetchContent_Declare(dlLoad
        GIT_REPOSITORY    https://github.com/Thomas-Chqt/dlLoad.git
        GIT_TAG           fd860b8200983b89d3ca58b371f6ec25b9972bd3
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(DL_BUILD_TESTS OFF)
    set(DL_INSTALL     ${GE_INSTALL})
    FetchContent_MakeAvailable(dlLoad)
    set_target_properties(dlLoad PROPERTIES FOLDER "dependencies")

    # -----------------------------
    # assimp
    # -----------------------------
    FetchContent_Declare(assimp
        GIT_REPOSITORY    https://github.com/assimp/assimp.git
        GIT_TAG           v5.4.1
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(ASSIMP_BUILD_TESTS                    OFF)
    set(ASSIMP_INSTALL                        OFF)
    set(ASSIMP_NO_EXPORT                      ON)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
    set(ASSIMP_BUILD_OBJ_IMPORTER             ON)
    set(ASSIMP_BUILD_FBX_IMPORTER             ON)
    set(ASSIMP_BUILD_GLTF_IMPORTER            ON)
    set(CMAKE_POLICY_DEFAULT_CMP0175 OLD)
    FetchContent_MakeAvailable(assimp)
    unset(CMAKE_POLICY_DEFAULT_CMP0175)
    if (assimp_SOURCE_DIR)
        if(TARGET zlibstatic OR TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
            set_target_properties(assimp PROPERTIES FOLDER "dependencies/assimp")
            if(TARGET zlibstatic)
                set_target_properties(zlibstatic PROPERTIES FOLDER "dependencies/assimp")
            endif()
            if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
                set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "dependencies/assimp")
            endif()
        else()
            set_target_properties(assimp PROPERTIES FOLDER "dependencies")
        endif()
    endif()

    # -----------------------------
    # yaml-cpp
    # -----------------------------
    FetchContent_Declare(yaml-cpp
        GIT_REPOSITORY    https://github.com/jbeder/yaml-cpp.git
        GIT_TAG           yaml-cpp-0.9.0
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(YAML_CPP_BUILD_TOOLS OFF)
    FetchContent_MakeAvailable(yaml-cpp)
    set_target_properties(yaml-cpp PROPERTIES FOLDER "dependencies")
endfunction()
