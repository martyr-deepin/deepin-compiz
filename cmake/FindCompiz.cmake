################################################################################
#
# FindCompiz
#
# This module finds Compiz (https://launchpad.net/compiz). It uses the FindPkgConfig to
# locate Compiz and adds the Compiz CMake module path to the CMake module path.
# It also loads the CompizDefaults that sets all variables to compile Compiz
# modules.
#
# This module sets the following variables:
#   COMPIZ_FOUND          ... set to true if compiz and its CompizDefaults macro
#                             is found
#
# Variables set by the FindPkgConfig macro for compiz
#   COMPIZ_CMAKE_MODULE_PATH ... the path containing all other
#				 compiz cmake modules
#   COMPIZ_LIBRARY_DIRS   ... the paths of the libraries (w/o the '-L')
#   COMPIZ_LDFLAGS        ... all required linker flags
#   COMPIZ_LDFLAGS_OTHER  ... all other linker flags
#   COMPIZ_INCLUDE_DIRS   ... the '-I' preprocessor flags (w/o the '-I')
#   COMPIZ_CFLAGS         ... all required cflags
#   COMPIZ_CFLAGS_OTHER   ... the other compiler flags
#   COMPIZ_VERSION        ... version of the module
#   COMPIZ_PREFIX         ... prefix-directory of the module
#   COMPIZ_INCLUDEDIR     ... include-dir of the module
#   COMPIZ_LIBDIR         ... lib-dir of the module
#
# If the _COMPIZ_INTERNAL variable is set to true, then this module will do
# nothing. This is required for the Compiz core package build system.
#
#  Author: Dennis Kasprzyk <onestone@compiz.org>
#
################################################################################

if (NOT _COMPIZ_INTERNAL)

    if (Compiz_FIND_REQUIRED)
	set (_req REQUIRED)
    endif ()

    # look for pkg-config
    find_package (PkgConfig ${_req})

    if (PKG_CONFIG_FOUND)

	# do we need to look for a specified version?
	set (_comp_ver)
	if (Compiz_FIND_VERSION)
	    if (Compiz_FIND_VERSION_EXACT)
		set (_comp_ver "=${Compiz_FIND_VERSION}")
	    else ()
		set (_comp_ver ">=${Compiz_FIND_VERSION}")
	    endif ()
	endif ()

	# add install prefix to pkgconfig search path if needed
        string (REGEX REPLACE "([\\+\\(\\)\\^\\\$\\.\\-\\*\\?\\|])" "\\\\\\1" PKGCONFIG_REGEX ${CMAKE_INSTALL_PREFIX})
	set (PKGCONFIG_REGEX ".*${PKGCONFIG_REGEX}/lib/pkgconfig:${PKGCONFIG_REGEX}/share/pkgconfig.*")

	if (NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

	    if (NOT "$ENV{PKG_CONFIG_PATH}" MATCHES "${PKGCONFIG_REGEX}")
		if ("" STREQUAL "$ENV{PKG_CONFIG_PATH}")
		    set (ENV{PKG_CONFIG_PATH} "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig:${CMAKE_INSTALL_PREFIX}/share/pkgconfig")
	    	else ()
		    set (ENV{PKG_CONFIG_PATH}
		         "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig:${CMAKE_INSTALL_PREFIX}/share/pkgconfig:$ENV{PKG_CONFIG_PATH}")
		endif ()
	    endif ()

	endif (NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

	# look for compiz
	pkg_check_modules (COMPIZ ${_req} "compiz${_comp_ver}")

	# COMPIZ_PREFIX is not set by default on all machines. The CMake docs
	# seem to vagely suggest this is normal in some cases for
	# pkg_check_modules.
	if (NOT COMPIZ_PREFIX)
	    set (COMPIZ_PREFIX ${CMAKE_INSTALL_PREFIX})
	endif ()

	# is the CompizDefaults module installed?
	find_path(_compiz_def_macro CompizDefaults.cmake ${COMPIZ_PREFIX}/share/compiz/cmake)

	if (COMPIZ_FOUND AND _compiz_def_macro)
            # everything found. Set module path and include defaults module
	    set (COMPIZ_CMAKE_MODULE_PATH ${COMPIZ_PREFIX}/share/compiz/cmake)
	    set (CMAKE_MODULE_PATH ${COMPIZ_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})
	    include (CompizDefaults)
	else ()
	    set (COMPIZ_FOUND 0)
	endif ()

        if (Compiz_FIND_REQUIRED AND NOT COMPIZ_FOUND)
	    message (FATAL_ERROR "Unable to find Compiz ${_comp_ver}")
	endif ()
    endif ()
endif ()

