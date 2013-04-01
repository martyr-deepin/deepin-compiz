#
# Main CMakeLists.txt for compiz
#
project (compiz)

set (CMAKE_MODULE_PATH_ORIG ${CMAKE_MODULE_PATH})
set (COMPIZ_CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${COMPIZ_CMAKE_MODULE_PATH})

include (CTest)

set (MEMORYCHECK_COMMAND_OPTIONS --trace-children=yes)
set (MEMORYCHECK_SUPPRESSIONS_FILE ${CMAKE_SOURCE_DIR}/tests/experimental-memcheck/compiz.supp CACHE FILEPATH "" FORCE)

include (Dart)

# Common compiz CMake macros & functions.
include (CompizCommon)
# Packaging-specific CMake macros & functions.
include (CompizPackage)

# GSettings schema recompilation
include (CompizGSettings)

include (cmake/base.cmake)
# Check if specific function is present in library.
include (CheckFunctionExists)

enable_testing()

# Import macro for doing coverage reporting
include(EnableCoverageReport)

#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
IF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
ENDIF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])

#set (CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRe" CACHE INTERNAL "" FORCE)
#if (NOT CMAKE_BUILD_TYPE)
#    set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug/Release/RelWithDebInfo/MinSizeRe)" FORCE)
#endif (NOT CMAKE_BUILD_TYPE)

file (READ ${CMAKE_SOURCE_DIR}/VERSION COMPIZ_RELEASE_VERSION LIMIT 12 OFFSET 0)
string (STRIP ${COMPIZ_RELEASE_VERSION} COMPIZ_RELEASE_VERSION)

set (VERSION ${COMPIZ_RELEASE_VERSION})

set (DECOR_INTERFACE_VERSION 20110504)

if (COMPIZ_PACKAGING_ENABLED)
    set (prefix ${CMAKE_INSTALL_PREFIX}                   CACHE PATH "prefix")
    set (exec_prefix ${CMAKE_INSTALL_PREFIX}/bin          CACHE PATH "bindir")
    set (libdir ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}  CACHE PATH "libdir")
    set (includedir ${CMAKE_INSTALL_PREFIX}/include       CACHE PATH "includedir")
    set (datadir ${CMAKE_INSTALL_PREFIX}/share            CACHE PATH "datadir")
else (COMPIZ_PACKAGING_ENABLED)
    set (prefix ${CMAKE_INSTALL_PREFIX}                 )
    set (exec_prefix ${CMAKE_INSTALL_PREFIX}/bin        )
    set (libdir ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
    set (includedir ${CMAKE_INSTALL_PREFIX}/include     )
    set (datadir ${CMAKE_INSTALL_PREFIX}/share          )
endif (COMPIZ_PACKAGING_ENABLED)

set (compiz_plugindir ${libdir}/compiz)
set (compiz_sharedir ${datadir}/compiz/)
set (compiz_metadatadir ${datadir}/compiz)
set (COMPIZ_I18N_DIR ${CMAKE_SOURCE_DIR}/po)

# Don't allow subdirectories to add schema recompile rule
set_property (GLOBAL
	      PROPERTY GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE
	      TRUE)
set_property (GLOBAL
	      PROPERTY COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE
	      TRUE)

# Translation
set (ALL_LINGUAS af ar bg bn bn_IN bs ca cs cy da de el en_GB en_US es eu et fi fr gl gu he hi hr hu id it ja ka km ko lo lt mk mr nb nl or pa pl pt pt_BR ro ru sk sl sr sv ta tr uk vi xh zh_CN zh_TW zu)
set (GETTEXT_PACKAGE compiz)

set (COMPIZ_REQUIRES
    x11
    xext
    xdamage
    xcomposite
    x11-xcb
    xrandr
    xinerama
    xext
    ice
    sm
    libxml-2.0
    libxslt
    glib-2.0
    gio-2.0
    glibmm-2.4
    "libstartup-notification-1.0 >= 0.7"
)

set (DECORATION_REQUIRES xrender)

compiz_pkg_check_modules (COMPIZ REQUIRED ${COMPIZ_REQUIRES})
compiz_pkg_check_modules (LIBDECORATION REQUIRED ${DECORATION_REQUIRES})

include (CompizDefaults)

set (_compiz_package_string "Compiz ${VERSION}")
set (_compiz_package "Compiz")

compiz_configure_file (${compiz_SOURCE_DIR}/config.h.core.in ${compiz_BINARY_DIR}/generated/config.h)

compiz_configure_file (
    ${CMAKE_SOURCE_DIR}/compiz.pc.in
    ${CMAKE_BINARY_DIR}/compiz.pc
    COMPIZ_REQUIRES
    COMPIZ_CFLAGS
)

install (
    FILES ${CMAKE_BINARY_DIR}/compiz.pc
    DESTINATION ${libdir}/pkgconfig
)

if (COMPIZ_BUILD_TESTING)

    # Build Google Test and make its headers known
    find_package (GTest)

    if (NOT GTEST_FOUND)

	# Check for google test and build it locally
	set (GTEST_ROOT_DIR
	     "/usr/src/gtest" # Default value, adjustable by user with e.g., ccmake
	     CACHE
	     PATH
	     "Path to Google Test srcs"
	     FORCE
	)

	find_path (GTEST_INCLUDE_DIR gtest/gtest.h)

	if (GTEST_INCLUDE_DIR)
	    add_subdirectory (${GTEST_ROOT_DIR}
			     gtest)
	endif (GTEST_INCLUDE_DIR)

	set (GTEST_BOTH_LIBRARIES gtest gtest_main)
	set (GTEST_FOUND TRUE)

    endif (NOT GTEST_FOUND)

    find_library (GMOCK_LIBRARY gmock)
    find_library (GMOCK_MAIN_LIBRARY gmock_main)

    if (NOT GMOCK_LIBRARY OR NOT GMOCK_MAIN_LIBRARY OR NOT GTEST_FOUND)

	message ("Google Mock and Google Test not found - cannot build tests!")
	set (COMPIZ_BUILD_TESTING OFF CACHE BOOL "" FORCE)

    else (NOT GMOCK_LIBRARY OR NOT GMOCK_MAIN_LIBRARY OR NOT GTEST_FOUND)

	include_directories (${GTEST_INCLUDE_DIRS})

    endif (NOT GMOCK_LIBRARY OR NOT GMOCK_MAIN_LIBRARY OR NOT GTEST_FOUND)

endif (COMPIZ_BUILD_TESTING)

find_package (Threads REQUIRED)

# Add the rest of compiz
add_subdirectory (cmake)
add_subdirectory (include)
add_subdirectory (images)
add_subdirectory (libdecoration)
add_subdirectory (gtk)
add_subdirectory (kde)
add_subdirectory (po)
add_subdirectory (metadata)
add_subdirectory (src)
add_subdirectory (compizconfig)
add_subdirectory (xslt)
add_subdirectory (plugins)
add_subdirectory (tests)

# Leave last - the cmake_install.cmake script is guarunteed
# to run in a single job, and we need to force GSettings
# schema recompilation to happen after all other install
# targets (there is no way to set install target dependencies)
# in CMake
add_subdirectory (postinst)

compiz_ensure_linkage ()
compiz_package_generation ("Compiz")
compiz_add_uninstall ()
compiz_add_git_dist ()
compiz_add_distcheck ()
compiz_add_release ()
compiz_add_release_signoff ()

if (COMPIZ_BUILD_TESTING)
    compiz_generate_testfile_target ()
endif (COMPIZ_BUILD_TESTING)

_print_configure_results ()

_check_compiz_cmake_macro (${CMAKE_MODULE_PATH_ORIG})

# Enable coverage reporting for compiz
compiz_generate_coverage_report()


