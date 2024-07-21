# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/06/20 16:36:49
# ---------------------------------------------------

add_executable(sandbox)

file(GLOB_RECURSE SANDBOX_SRC "src/*")
target_sources(sandbox PRIVATE ${SANDBOX_SRC})
target_include_directories(sandbox PRIVATE "src")
target_link_libraries(sandbox PRIVATE Game-Engine)
