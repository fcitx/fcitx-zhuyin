# - Try to find the Libpinyin libraries
# Once done this will define
#
#  LIBZHUYIN_FOUND - system has LIBZHUYIN
#  LIBZHUYIN_INCLUDE_DIR - the LIBZHUYIN include directory
#  LIBZHUYIN_LIBRARIES - LIBZHUYIN library
#
# Copyright (c) 2012 CSSlayer <wengxt@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(LIBZHUYIN_INCLUDE_DIR AND LIBZHUYIN_LIBRARIES)
    # Already in cache, be silent
    set(LIBZHUYIN_FIND_QUIETLY TRUE)
endif(LIBZHUYIN_INCLUDE_DIR AND LIBZHUYIN_LIBRARIES)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBZHUYIN "libzhuyin>=0.9.91")
_pkgconfig_invoke("libzhuyin" PC_LIBZHUYIN LIBZHUYININCLUDEDIR "" "--variable=libzhuyinincludedir")
_pkgconfig_invoke("libzhuyin" LIBZHUYIN PKGDATADIR "" "--variable=pkgdatadir")
_pkgconfig_invoke("libzhuyin" LIBZHUYIN EXECPREFIX "" "--variable=exec_prefix")

find_path(LIBZHUYIN_INCLUDE_DIR
          NAMES zhuyin.h
          HINTS ${PC_LIBZHUYIN_LIBZHUYININCLUDEDIR})

find_library(LIBZHUYIN_LIBRARIES
             NAMES zhuyin
             HINTS ${PC_LIBZHUYIN_LIBDIR})

find_program(LIBZHUYIN_GEN_BINARY_FILES gen_binary_files HINTS "${LIBZHUYIN_EXECPREFIX}/bin")
find_program(LIBZHUYIN_GEN_UNIGRAM gen_unigram HINTS "${LIBZHUYIN_EXECPREFIX}/bin")
find_program(LIBZHUYIN_IMPORT_INTERPOLATION import_interpolation HINTS "${LIBZHUYIN_EXECPREFIX}/bin")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibZhuyin  DEFAULT_MSG
                                  LIBZHUYIN_LIBRARIES
                                  LIBZHUYIN_INCLUDE_DIR
                                  LIBZHUYIN_PKGDATADIR
                                  LIBZHUYIN_GEN_BINARY_FILES
                                  LIBZHUYIN_GEN_UNIGRAM
                                  LIBZHUYIN_IMPORT_INTERPOLATION
                                  PC_LIBZHUYIN_FOUND
                                  )

mark_as_advanced(LIBZHUYIN_INCLUDE_DIR LIBZHUYIN_LIBRARIES)
