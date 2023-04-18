cmake_minimum_required(VERSION 3.6)

include(FetchContent)
include(CheckCCompilerFlag)
include("tools/glsl-shaders.cmake")

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

project(vkhandybug VERSION "0.1.2")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/frontend/configure.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/frontend/configure.h")

find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)

include("tools/cmake.dependencies")

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(vkmemalloc)
FetchContent_MakeAvailable(vkbootstrap)
FetchContent_MakeAvailable(fmt)
FetchContent_Populate(miniaudio)
FetchContent_Populate(cereal)
FetchContent_Populate(imgui)
FetchContent_Populate(imgui_filedialog)
FetchContent_Populate(hashlib)

file(GLOB_RECURSE vkhandybug_SOURCES CONFIGURE_DEPENDS 
    "frontend/*.c" 
    "frontend/*.cpp" 
    "frontend/renderer/*.cpp" 
    "frontend/renderer/*.c" 
    "frontend/resources/shaders/*.spv" 
    "lynx_core/*.cpp"    
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_filedialog_SOURCE_DIR}/ImGuiFileDialog.cpp  
    ${hashlib_SOURCE_DIR}/md5.cpp  
)

FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
  FetchContent_Populate(zlib)
  add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(${PROJECT_NAME} ${vkhandybug_SOURCES})

check_c_compiler_flag(-Wcpp HAVE_WARNING)
check_c_compiler_flag(-Wreturn-type HAVE_RETURN_TYPE)

if (HAVE_WARNING)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-cpp)
endif (HAVE_WARNING)

if (HAVE_RETURN_TYPE)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-return-type)
endif (HAVE_RETURN_TYPE)

target_glsl_shaders( ${PROJECT_NAME} PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}/frontend/resources/shaders/lynx_render.comp"
)
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${Vulkan_INCLUDE_DIRS}
    ${vkbootstrap_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${vkmemalloc_SOURCE_DIR}/include
    ${zlib_SOURCE_DIR}
    ${zlib_BINARY_DIR}
    ${fmt_SOURCE_DIR}/include
    ${imgui_filedialog_SOURCE_DIR}
    ${miniaudio_SOURCE_DIR}
    ${cereal_SOURCE_DIR}/include
    ${hashlib_SOURCE_DIR}
    "lynx_core/"
    "frontend/include/"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    Threads::Threads
    vk-bootstrap::vk-bootstrap
    VulkanMemoryAllocator
    glfw
    zlibstatic
    fmt
    )

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/frontend/resources/shaders/lynx_render.comp.spv" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/lynx_render.comp.spv" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/frontend/resources/fonts/Crisp.ttf" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fonts/Crisp.ttf" COPYONLY)
