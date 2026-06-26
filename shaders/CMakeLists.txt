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
    DEPENDS gfxsc ${TEXTURED_SSRC} "FrameData.slang" "Light.slang" "phong_shading.slang" "shader.h"
    COMMENT "Building textured shader"
    VERBATIM
)
add_custom_target(textured_shader ALL DEPENDS ${TEXTURED_SLIB})
set_target_properties(textured_shader PROPERTIES FOLDER "shaders")

set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
