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
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

project(vkhandybug VERSION "0.6.4")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/frontend/configure.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/frontend/configure.h")

find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)

include("tools/cmake.dependencies")

set(luasrcFiles ${lua_SOURCE_DIR}/src/lapi.c ${lua_SOURCE_DIR}/src/lauxlib.c ${lua_SOURCE_DIR}/src/lbaselib.c ${lua_SOURCE_DIR}/src/lcode.c ${lua_SOURCE_DIR}/src/lcorolib.c ${lua_SOURCE_DIR}/src/lctype.c ${lua_SOURCE_DIR}/src/ldblib.c ${lua_SOURCE_DIR}/src/ldebug.c ${lua_SOURCE_DIR}/src/ldo.c
${lua_SOURCE_DIR}/src/ldump.c ${lua_SOURCE_DIR}/src/lfunc.c ${lua_SOURCE_DIR}/src/lgc.c ${lua_SOURCE_DIR}/src/linit.c ${lua_SOURCE_DIR}/src/liolib.c ${lua_SOURCE_DIR}/src/llex.c ${lua_SOURCE_DIR}/src/lmathlib.c ${lua_SOURCE_DIR}/src/lmem.c ${lua_SOURCE_DIR}/src/loadlib.c ${lua_SOURCE_DIR}/src/lobject.c ${lua_SOURCE_DIR}/src/lopcodes.c
    ${lua_SOURCE_DIR}/src/loslib.c ${lua_SOURCE_DIR}/src/lparser.c ${lua_SOURCE_DIR}/src/lstate.c ${lua_SOURCE_DIR}/src/lstring.c ${lua_SOURCE_DIR}/src/lstrlib.c ${lua_SOURCE_DIR}/src/ltable.c ${lua_SOURCE_DIR}/src/ltablib.c ${lua_SOURCE_DIR}/src/ltests.c ${lua_SOURCE_DIR}/src/ltm.c ${lua_SOURCE_DIR}/src/lundump.c
    ${lua_SOURCE_DIR}/src/lutf8lib.c ${lua_SOURCE_DIR}/src/lvm.c ${lua_SOURCE_DIR}/src/lzio.c)

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
    ${luasrcFiles}
)

FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
  FetchContent_Populate(zlib)
  add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
  FetchContent_Populate(glfw)
  add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)
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

target_glsl_shaders(${PROJECT_NAME} PRIVATE
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
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/src
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

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fonts DESTINATION bin)
install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders DESTINATION bin)

add_custom_target(
    ${PROJECT_NAME}.AppImage
    COMMAND ${CMAKE_COMMAND}
    ARGS -P "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_appimage.cmake"
    DEPENDS ${PROJECT_NAME}
    )
