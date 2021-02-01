
#- Find LibAvif
# Find the LibAvif library <https://github.com/AOMediaCodec/libavif/>
# This module defines
#  LibAvif_VERSION_STRING, the version string of LibAvif
#  LibAvif_INCLUDE_DIR, where to find libraw.h
#  LibAvif_LIBRARIES, the libraries needed to use LibAvif (non-thread-safe)
#  LibAvif_r_LIBRARIES, the libraries needed to use LibAvif (thread-safe)
#  LibAvif_DEFINITIONS, the definitions needed to use LibAvif (non-thread-safe)
#  LibAvif_r_DEFINITIONS, the definitions needed to use LibAvif (thread-safe)
#
# Copyright (c) 2013, Pino Toscano <pino at kde dot org>
# Copyright (c) 2013, Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

FIND_PACKAGE(PkgConfig)

IF (PKG_CONFIG_FOUND AND NOT LIBAVIF_PATH)
    PKG_CHECK_MODULES(PC_LIBAVIF QUIET libavif)
    SET(LibAvif_DEFINITIONS ${PC_LIBAVIF_CFLAGS_OTHER})

    PKG_CHECK_MODULES(PC_LIBAVIF_R QUIET libavif_r)
    SET(LibAvif_r_DEFINITIONS ${PC_LIBAVIF_R_CFLAGS_OTHER})
ENDIF ()

FIND_PATH(LibAvif_INCLUDE_DIR avif/avif.h
        HINTS
        ${LIBAVIF_PATH}
        ${PC_LIBAVIF_INCLUDEDIR}
        ${PC_LibAvif_INCLUDE_DIRS}
        PATH_SUFFIXES libavif
        )

FIND_LIBRARY(LibAvif_LIBRARIES NAMES avif
        HINTS
        ${LIBAVIF_PATH}
        ${PC_LIBAVIF_LIBDIR}
        ${PC_LIBAVIF_LIBRARY_DIRS}
        )

FIND_LIBRARY(LibAvif_r_LIBRARIES NAMES avif_r
        HINTS
        ${LIBAVIF_PATH}
        ${PC_LIBAVIF_R_LIBDIR}
        ${PC_LIBAVIF_R_LIBRARY_DIRS}
        )

IF (LibAvif_INCLUDE_DIR)
    FILE(READ ${LibAvif_INCLUDE_DIR}/avif/avif.h _libavif_version_content)

    STRING(REGEX MATCH "#define AVIF_VERSION_MAJOR[ \t]*([0-9]*)\n" _version_major_match ${_libavif_version_content})
    SET(_libavif_version_major "${CMAKE_MATCH_1}")

    STRING(REGEX MATCH "#define AVIF_VERSION_MINOR[ \t]*([0-9]*)\n" _version_minor_match ${_libavif_version_content})
    SET(_libavif_version_minor "${CMAKE_MATCH_1}")

    STRING(REGEX MATCH "#define AVIF_VERSION_PATCH[ \t]*([0-9]*)\n" _version_patch_match ${_libavif_version_content})
    SET(_libavif_version_patch "${CMAKE_MATCH_1}")

    IF (_version_major_match AND _version_minor_match AND _version_patch_match)
        SET(LibAvif_VERSION_STRING "${_libavif_version_major}.${_libavif_version_minor}.${_libavif_version_patch}")
    ELSE ()
        IF (NOT LibAvif_FIND_QUIETLY)
            MESSAGE(STATUS "Failed to get version information from ${LibAvif_INCLUDE_DIR}/avif/avif.h")
        ENDIF ()
    ENDIF ()
ENDIF ()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibAvif
        REQUIRED_VARS LibAvif_LIBRARIES LibAvif_INCLUDE_DIR
        VERSION_VAR LibAvif_VERSION_STRING
        )

MARK_AS_ADVANCED(LibAvif_VERSION_STRING
        LibAvif_INCLUDE_DIR
        LibAvif_LIBRARIES
        LibAvif_r_LIBRARIES
        LibAvif_DEFINITIONS
        LibAvif_r_DEFINITIONS
        )