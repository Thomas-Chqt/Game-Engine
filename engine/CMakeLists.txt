# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/09/10 10:01:04
# ---------------------------------------------------

include(FetchContent)

add_library(Game-Engine STATIC)
set_target_properties(Game-Engine PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE GAME_ENGINE_SRC "*.cpp" "*.hpp")
target_sources(Game-Engine PRIVATE ${GAME_ENGINE_SRC})

target_include_directories(Game-Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(GE_BUILD_METAL)
    set(MTL_SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders")
    set(MTL_SHADER_LIB "${CMAKE_BINARY_DIR}/MTLShaderLib.metallib")
endif()
if(GE_BUILD_OPENGL)
    set(OPENGL_SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders")
endif()

if(GE_BUILD_METAL)
    target_compile_definitions(Game-Engine PRIVATE "MTL_SHADER_LIB=\"${MTL_SHADER_LIB}\"")
endif()
if(GE_BUILD_OPENGL)
    target_compile_definitions(Game-Engine PRIVATE "OPENGL_SHADER_DIR=\"${OPENGL_SHADER_DIR}\"")
endif()

target_compile_definitions(Game-Engine PUBLIC "UTILSCPP_API_EXPORT")
target_compile_definitions(Game-Engine PUBLIC "MATH_API_EXPORT")
target_compile_definitions(Game-Engine PUBLIC "GAME_ENGINE_API_EXPORT")

target_link_libraries(Game-Engine PUBLIC  UtilsCPP)
target_link_libraries(Game-Engine PUBLIC  Math)
target_link_libraries(Game-Engine PUBLIC  Graphics)
target_link_libraries(Game-Engine PRIVATE assimp)
target_link_libraries(Game-Engine PUBLIC  nlohmann_json::nlohmann_json)
target_link_libraries(Game-Engine PUBLIC  stduuid)
target_link_libraries(Game-Engine PUBLIC  stb_image)
target_link_libraries(Game-Engine PUBLIC  dlLoad)

if(GE_BUILD_METAL)
    add_custom_target(MTLShaderLib DEPENDS ${MTL_SHADER_LIB})
    
    file(GLOB MTL_SHADER_LIB_SRCS "${MTL_SHADER_DIR}/*.metal")
    target_sources(MTLShaderLib PRIVATE ${MTL_SHADER_LIB_SRCS})
    
    add_dependencies(Game-Engine MTLShaderLib)
    
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        list(APPEND MTL_SHADER_BUILD_FLAGS "-gline-tables-only" "-frecord-sources")
    endif()
    
    find_program(XCRUN_PATH xcrun REQUIRED)
    
    foreach(src IN LISTS MTL_SHADER_LIB_SRCS)
        string(REPLACE ".metal" ".air" air "${src}")
        string(REPLACE ${MTL_SHADER_DIR} ${CMAKE_CURRENT_BINARY_DIR} air "${air}")
        add_custom_command(OUTPUT "${air}" COMMAND ${XCRUN_PATH} -sdk macosx metal ${MTL_SHADER_BUILD_FLAGS} -o "${air}" -c "${src}" DEPENDS "${src}")
        list(APPEND MTL_SHADER_LIB_AIRS "${air}")
    endforeach()
    
    add_custom_command(OUTPUT ${MTL_SHADER_LIB} COMMAND ${XCRUN_PATH} -sdk macosx metallib ${MTL_SHADER_LINK_FLAGS} -o ${MTL_SHADER_LIB} ${MTL_SHADER_LIB_AIRS} DEPENDS ${MTL_SHADER_LIB_AIRS})
endif()