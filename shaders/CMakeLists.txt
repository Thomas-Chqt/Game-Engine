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

set(FLAT_COLOR_SSRC "${CMAKE_CURRENT_SOURCE_DIR}/flat_color.slang")
set(FLAT_COLOR_SLIB ${CMAKE_CURRENT_BINARY_DIR}/flat_color.slib)
add_custom_command(
    OUTPUT ${FLAT_COLOR_SLIB}
    COMMAND $<TARGET_FILE:gfxsc> -t ${SHADER_TARGETS} -o ${FLAT_COLOR_SLIB} ${FLAT_COLOR_SSRC}
    DEPENDS gfxsc ${FLAT_COLOR_SSRC} "FrameData.slang" "Light.slang" "phong_shading.slang" "shader.h"
    COMMENT "Building flat_color shader"
    VERBATIM
)
add_custom_target(flat_color_shader ALL DEPENDS ${FLAT_COLOR_SLIB})
set_target_properties(flat_color_shader PROPERTIES FOLDER "shaders")

set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
