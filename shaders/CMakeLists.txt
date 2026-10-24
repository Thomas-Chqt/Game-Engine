# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# ---------------------------------------------------

set(SHADER_TARGET_LIST)
if (GE_BUILD_METAL)
    list(APPEND SHADER_TARGET_LIST metal)
endif()
if (GE_BUILD_VULKAN)
    list(APPEND SHADER_TARGET_LIST spirv)
endif()
list(JOIN SHADER_TARGET_LIST "," SHADER_TARGETS)

set(TEXTURED_SSRC "${CMAKE_CURRENT_SOURCE_DIR}/textured.slang")
set(TEXTURED_SLIB ${CMAKE_CURRENT_BINARY_DIR}/textured.slib)
add_custom_command(
    OUTPUT ${TEXTURED_SLIB}
    COMMAND $<TARGET_FILE:gfxsc> -t ${SHADER_TARGETS} -o ${TEXTURED_SLIB} ${TEXTURED_SSRC}
    DEPENDS gfxsc ${TEXTURED_SSRC} "FrameData.slang" "Light.slang" "pbr.slang" "brdf.slang" "ibl.slang" "utils.slang" "shader.h"
    COMMENT "Building textured shader"
    VERBATIM
)
add_custom_target(textured_shader ALL DEPENDS ${TEXTURED_SLIB})
set_target_properties(textured_shader PROPERTIES FOLDER "shaders")

set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

foreach(ENTRY_POINT equirectangularToCubemap convolveDiffuseIrradiance prefilterEnvironment integrateBrdf)
    set(ENTRY_POINT_SLIB "${CMAKE_CURRENT_BINARY_DIR}/${ENTRY_POINT}.slib")
    add_custom_command(
        OUTPUT ${ENTRY_POINT_SLIB}
        COMMAND $<TARGET_FILE:gfxsc> -t ${SHADER_TARGETS} -e ${ENTRY_POINT} -o ${ENTRY_POINT_SLIB} "${CMAKE_CURRENT_SOURCE_DIR}/pbr_precompute.slang"
        DEPENDS gfxsc pbr_precompute.slang brdf.slang utils.slang shader.h
        VERBATIM
    )
    list(APPEND PBR_PRECOMPUTE_SLIBS ${ENTRY_POINT_SLIB})
endforeach()
add_custom_target(pbr_precompute_shaders ALL DEPENDS ${PBR_PRECOMPUTE_SLIBS})
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/skybox.slib"
    COMMAND $<TARGET_FILE:gfxsc> -t ${SHADER_TARGETS} -o "${CMAKE_CURRENT_BINARY_DIR}/skybox.slib" "${CMAKE_CURRENT_SOURCE_DIR}/skybox.slang"
    DEPENDS gfxsc skybox.slang utils.slang shader.h
    VERBATIM
)
add_custom_target(skybox_shader ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/skybox.slib")
set_target_properties(pbr_precompute_shaders skybox_shader PROPERTIES FOLDER "shaders")
