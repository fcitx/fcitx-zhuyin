cmake_minimum_required(VERSION 2.8.6)
project(fcitx-zhuyin)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++11" SUPPORT_CXX11)

if(NOT SUPPORT_CXX11)
    message(FATAL_ERROR "need c++ 11 compatible compiler to compile")
endif()

find_package(Libintl REQUIRED)
find_package(Fcitx 4.2.8 REQUIRED)
find_package(GLIB2 REQUIRED)
find_package(LibZhuyin REQUIRED)

_fcitx_add_uninstall_target()

set(CMAKE_C_FLAGS "-Wall -Wextra -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11 -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined -Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_MODULE_LINKER_FLAGS}")

set(FCITX_LIBZHUYIN_ZHUYIN_DATADIR
  "${FCITX4_PREFIX}/share/fcitx/libzhuyin/zhuyin_data")

configure_file(config.h.in config.h)

add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)
