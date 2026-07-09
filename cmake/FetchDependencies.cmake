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
        GIT_TAG           b96b96a9e5679a4d318c3d5bdd90dfb4174fed42
        GIT_SHALLOW       0
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(GFX_BUILD_METAL      ${GE_BUILD_METAL})
    set(GFX_BUILD_VULKAN     ${GE_BUILD_VULKAN})
    set(GFX_ENABLE_IMGUI     ON)
    set(GFX_ENABLE_GLFW      ON)
    set(GFX_BUILD_EXAMPLES   OFF)
    set(GFX_BUILD_TESTS      OFF)
    set(GFX_BUILD_TRACY      ON)
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
        GIT_SHALLOW       0
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
        GIT_SHALLOW       0
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
        GIT_SHALLOW       0
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(DL_BUILD_TESTS OFF)
    set(DL_INSTALL     ${GE_INSTALL})
    FetchContent_MakeAvailable(dlLoad)
    set_target_properties(dlLoad PROPERTIES FOLDER "dependencies")

    # -----------------------------
    # fastgltf
    # -----------------------------
    FetchContent_Declare(fastgltf
        GIT_REPOSITORY    https://github.com/spnda/fastgltf.git
        GIT_TAG           v0.8.0
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(FASTGLTF_COMPILE_AS_CPP20 ON)
    set(FASTGLTF_ENABLE_TESTS     OFF)
    set(FASTGLTF_ENABLE_EXAMPLES  OFF)
    set(FASTGLTF_ENABLE_DOCS      OFF)
    FetchContent_MakeAvailable(fastgltf)
    if (TARGET fastgltf)
        set_target_properties(fastgltf PROPERTIES FOLDER "dependencies")
    endif()

    # -----------------------------
    # ImGuizmo
    # -----------------------------
    FetchContent_Declare(imguizmo
        GIT_REPOSITORY    https://github.com/CedricGuillemet/ImGuizmo.git
        GIT_TAG           1.10
        GIT_SHALLOW       1
        GIT_PROGRESS      TRUE
        FIND_PACKAGE_ARGS
    )
    set(IMGUIZMO_BUILD_EXAMPLE OFF)
    FetchContent_MakeAvailable(imguizmo)
    set_target_properties(imguizmo PROPERTIES FOLDER "dependencies")
    target_link_libraries(imguizmo PRIVATE imgui)
endfunction()
