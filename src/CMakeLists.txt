
include_directories (
    ${PROJECT_BINARY_DIR}
    ${LIBZHUYIN_INCLUDE_DIR}
    ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
    ${FCITX4_FCITX_CONFIG_INCLUDE_DIRS}
    ${FCITX4_FCITX_INCLUDE_DIRS}
    ${LIBINTL_INCLUDE_DIR}
    ${GLIB2_INCLUDE_DIR}
    )

link_directories (${LIBZHUYIN_LIBRARY_DIRS})

set( fcitx_zhuyin_sources
    eim.cpp
    enummap.cpp
    config.cpp
    utils.cpp
    )

add_definitions( -DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\" )

fcitx_add_addon_full(zhuyin DESC
SOURCES ${fcitx_zhuyin_sources}
IM_CONFIG zhuyin.conf
LINK_LIBS ${LIBZHUYIN_LIBRARIES} ${LIBINTL_LIBRARIES} ${GLIB2_LIBRARIES}
)
