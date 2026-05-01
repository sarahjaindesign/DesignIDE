cmake_minimum_required(VERSION 3.20)
project(DesignIDE VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# GLFW - window creation
include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.9
)
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(glfw)

# Dear ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
add_library(imgui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC 
  ${IMGUI_DIR} 
  ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw opengl32)

# DesignIDE executable
add_executable(DesignIDE 
    src/main.cpp
    src/checks/accessibility.cpp
    src/checks/consistency.cpp
        src/checks/handoff.cpp

)

target_include_directories(DesignIDE PRIVATE
  ${CMAKE_SOURCE_DIR}/third_party
  ${CMAKE_SOURCE_DIR}/third_party/imgui
  ${CMAKE_SOURCE_DIR}/third_party/imgui/backends
)

target_link_libraries(DesignIDE PRIVATE imgui glfw opengl32)