Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
  miniaudio
  GIT_REPOSITORY       https://github.com/mackron/miniaudio.git
  GIT_TAG              0.11.15
  GIT_SHALLOW          TRUE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY       https://github.com/ocornut/imgui.git
  GIT_TAG              dc3e531ff28450bff73fde0163b1d076b6bb5605
  GIT_SHALLOW          FALSE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY       https://github.com/glfw/glfw.git
  GIT_TAG              3.3.8
  GIT_SHALLOW          TRUE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  vkmemalloc
  GIT_REPOSITORY       https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG              v3.0.1
  GIT_SHALLOW          TRUE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  vkbootstrap
  GIT_REPOSITORY       https://github.com/charles-lunarg/vk-bootstrap
  GIT_TAG              61f77612c70dd49a59157fe139a7d248a90e206a
  GIT_SHALLOW          TRUE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  zlib
  GIT_REPOSITORY       https://github.com/madler/zlib.git
  GIT_TAG              04f42ceca40f73e2978b50e93806c2a18c1281fc
  GIT_SHALLOW          TRUE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  libzip
  GIT_REPOSITORY       https://github.com/nih-at/libzip.git
  GIT_TAG              v1.10.0
  GIT_SHALLOW          TRUE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  imgui_filedialog
  GIT_REPOSITORY       https://github.com/aiekick/ImGuiFileDialog.git
  GIT_TAG              v0.6.5
  GIT_SHALLOW          TRUE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  cereal
  GIT_REPOSITORY       https://github.com/USCiLab/cereal.git
  GIT_TAG              ddd467244713ea4fe63733628992efcdd6a9187d
  GIT_SHALLOW          FALSE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY       https://github.com/fmtlib/fmt.git
  GIT_TAG              9.1.0
  GIT_SHALLOW          TRUE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  hashlib
  GIT_REPOSITORY       https://github.com/stbrumme/hash-library.git
  GIT_TAG              d389d18112bcf7e4786ec5e8723f3658a7f433d7
  GIT_SHALLOW          FALSE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  sol2
  GIT_REPOSITORY       https://github.com/ThePhD/sol2.git
  GIT_TAG              v3.3.0
  GIT_SHALLOW          TRUE
  GIT_PROGRESS         TRUE
)

FetchContent_Declare(
  lua
  URL       https://www.lua.org/ftp/lua-5.4.6.tar.gz
  URL_HASH  SHA256=7d5ea1b9cb6aa0b59ca3dde1c6adcb57ef83a1ba8e5432c0ecd06bf439b3ad88
)

FetchContent_MakeAvailable(vkmemalloc)
FetchContent_MakeAvailable(vkbootstrap)
FetchContent_MakeAvailable(fmt)
FetchContent_Populate(lua)
FetchContent_Populate(sol2)
FetchContent_Populate(miniaudio)
FetchContent_Populate(cereal)
FetchContent_Populate(imgui)
FetchContent_Populate(imgui_filedialog)
FetchContent_Populate(hashlib)

FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
  FetchContent_Populate(glfw)
  add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

find_package(ZLIB)
if(NOT ZLIB_FOUND)
  FetchContent_Populate(zlib)
  add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} EXCLUDE_FROM_ALL)
  configure_file("${zlib_BINARY_DIR}/zconf.h" "${zlib_SOURCE_DIR}/zconf.h" COPYONLY)
  set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR})  
  if(MSVC)
    set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif()
  set(ZLIB_INTERN ON)
endif()

if(NOT libzip_POPULATED)
  FetchContent_Populate(libzip)
  set(BUILD_TOOLS OFF CACHE BOOL "Build tools in the src directory (zipcmp, zipmerge, ziptool)" FORCE)
  set(BUILD_REGRESS OFF CACHE BOOL "Build regression tests" FORCE)
  set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
  set(BUILD_DOC OFF CACHE BOOL "Build documentation" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
  set(LIBZIP_DO_INSTALL OFF CACHE BOOL "Install libzip and the related files" FORCE)
  add_subdirectory(${libzip_SOURCE_DIR} ${libzip_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

if(NOT APPLE)
  file(READ "${FETCHCONTENT_BASE_DIR}/vkmemalloc-src/src/CMakeLists.txt" FILE_CONTENTS)
  string(REPLACE "CXX_STANDARD 14" "CXX_STANDARD 20" FILE_CONTENTS "${FILE_CONTENTS}")
  file(WRITE "${FETCHCONTENT_BASE_DIR}/vkmemalloc-src/src/CMakeLists.txt" "${FILE_CONTENTS}")

  #miniaudio MSVC
  file(READ "${FETCHCONTENT_BASE_DIR}/miniaudio-src/miniaudio.h" FILE_CONTENTS)
  string(REPLACE "sizeof(pInfo->name), 0, FALSE)" "sizeof(pInfo->name), 0, 0)" FILE_CONTENTS "${FILE_CONTENTS}")
  string(REPLACE "sizeof(pData->deviceName), 0, FALSE)" "sizeof(pData->deviceName), 0, 0)" FILE_CONTENTS "${FILE_CONTENTS}")
  string(REPLACE "sizeof(guidStr), 0, FALSE)" "sizeof(guidStr), 0, 0)" FILE_CONTENTS "${FILE_CONTENTS}")
  file(WRITE "${FETCHCONTENT_BASE_DIR}/miniaudio-src/miniaudio.h" "${FILE_CONTENTS}")
else()
  file(READ "${FETCHCONTENT_BASE_DIR}/hashlib-src/md5.cpp" FILE_CONTENTS)
  string(REPLACE "#ifndef _MSC_VER" "#if !defined(_MSC_VER) && !defined(__APPLE__)" FILE_CONTENTS "${FILE_CONTENTS}")
  file(WRITE "${FETCHCONTENT_BASE_DIR}/hashlib-src/md5.cpp" "${FILE_CONTENTS}")
endif()
