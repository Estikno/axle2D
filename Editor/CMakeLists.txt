# ══════════════════════════════════════════════════════════════════════════════
#  Editor/CMakeLists.txt
# ══════════════════════════════════════════════════════════════════════════════

set(VENDOR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")

set(INC_IMGUIZMO           "${VENDOR}/ImGuizmo/src")

# ══════════════════════════════════════════════════════════════════════════════
#  ImGuizmo (C++ ImGui static lib)
# ══════════════════════════════════════════════════════════════════════════════
add_library(ImGuizmo STATIC
    "${VENDOR}/ImGuizmo/src/ImGuizmo.cpp"
    "${VENDOR}/ImGuizmo/src/ImGuizmo.h"
)
target_include_directories(ImGuizmo PUBLIC "${INC_IMGUIZMO}")
target_link_libraries(ImGuizmo PUBLIC ImGui)   # propagates INC_IMGUI too

if(UNIX)
    set_target_properties(ImGuizmo PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# ══════════════════════════════════════════════════════════════════════════════
#  Editor (C++20 Executable)
# ══════════════════════════════════════════════════════════════════════════════
file(GLOB_RECURSE EDITOR_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.h"
)

add_executable(Editor ${EDITOR_SOURCES})

target_include_directories(Editor
    PRIVATE
        "${CMAKE_SOURCE_DIR}/Axle/src"
        ${AXLE_VENDOR_INCLUDES}   # set by Axle/CMakeLists.txt
)

target_compile_definitions(Editor
    PRIVATE
        $<$<CONFIG:Debug>:AX_DEBUG>
        $<$<CONFIG:Release>:AX_RELEASE>
        $<$<CONFIG:Dist>:AX_DIST>
)

# Platform defines
if(WIN32)
    target_compile_definitions(Editor PRIVATE AX_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(Editor PRIVATE AX_PLATFORM_LINUX)
endif()

target_link_libraries(Editor PRIVATE Axle ImGui ImGuizmo)

set_target_properties(Editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}/$<CONFIG>/${CMAKE_SYSTEM_NAME}-x64/Editor"
)

target_compile_options(Editor PRIVATE
    -fno-exceptions
)
