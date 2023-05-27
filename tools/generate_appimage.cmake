set(EXE "${CMAKE_BINARY_DIR}/bin/vkhandybug")
set(NAME "VkHandyBug")
set(ICON "${CMAKE_CURRENT_SOURCE_DIR}/../assets/icon.png")
set(DIR_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../assets/icon.png")
set(OUTPUT_NAME "${CMAKE_BINARY_DIR}/bin/vkhandybug.AppImage")

SET(AIT_PATH "${CMAKE_BINARY_DIR}/AppImageTool.AppImage" CACHE INTERNAL "")
if (NOT EXISTS "${AIT_PATH}")
    file(DOWNLOAD https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage "${AIT_PATH}")
    execute_process(COMMAND chmod +x ${AIT_PATH})
endif()

set(APPDIR "${CMAKE_BINARY_DIR}/AppDir")
file(REMOVE_RECURSE "${APPDIR}")
file(MAKE_DIRECTORY "${APPDIR}")

file(COPY "${EXE}" DESTINATION "${APPDIR}" FOLLOW_SYMLINK_CHAIN)
get_filename_component(NAME "${EXE}" NAME)

file(WRITE "${APPDIR}/AppRun" 
"#!/bin/sh
cd \"$(dirname \"$0\")\";
./${NAME} $@"
)
execute_process(COMMAND chmod +x "${APPDIR}/AppRun")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../frontend/resources/shaders/lynx_render.comp.spv" DESTINATION "${APPDIR}/shaders")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../frontend/resources/fonts/Crisp.ttf" DESTINATION "${APPDIR}/fonts")

file(COPY ${DIR_ICON} DESTINATION "${APPDIR}")
get_filename_component(THUMB_NAME "${DIR_ICON}" NAME)
file(RENAME "${APPDIR}/${THUMB_NAME}" "${APPDIR}/.DirIcon")

file(COPY ${ICON} DESTINATION "${APPDIR}")
get_filename_component(ICON_NAME "${ICON}" NAME)
get_filename_component(ICON_EXT "${ICON}" EXT)
file(RENAME "${APPDIR}/${ICON_NAME}" "${APPDIR}/${NAME}${ICON_EXT}")

file(WRITE "${APPDIR}/${NAME}.desktop" 
"[Desktop Entry]
Type=Application
Name=${NAME}
Icon=${NAME}
Categories=X-None;"    
)

execute_process(COMMAND ${AIT_PATH} ${APPDIR} ${OUTPUT_NAME})
file(REMOVE_RECURSE "${APPDIR}")