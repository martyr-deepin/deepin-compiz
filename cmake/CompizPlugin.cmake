#######################################################################
#
# Generic Compiz Fusion plugin cmake module
#
# Copyright : (C) 2008 by Dennis Kasprzyk
# E-mail    : onestone@opencompositing.org
#
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
#######################################################################
#
# This module provides the following macro:
#
# compiz_plugin (<plugin name>
#                       [PKGDEPS dep1 dep2 ...]
#                       [PLUGINDEPS plugin1 plugin2 ...]
#                       [LDFLAGSADD flag1 flag2 ...]
#                       [CFLAGSADD flag1 flag2 ...]
#                       [LIBRARIES lib1 lib2 ...]
#                       [LIBDIRS dir1 dir2 ...]
#                       [INCDIRS dir1 dir2 ...])
#
# PKGDEPS    = pkgconfig dependencies
# PLUGINDEPS = compiz plugin dependencies
# LDFLAGSADD = flags added to the link command
# CFLAGSADD  = flags added to the compile command
# LIBRARIES  = libraries added to link command
# LIBDIRS    = additional link directories
# INCDIRS    = additional include directories
#
# The following variables will be used by this macro:
#
# BUILD_GLOBAL=true Environment variable to install a plugin
#                   into the compiz directories
#
# COMPIZ_PLUGIN_INSTALL_TYPE = (package | compiz | local (default))
#     package = Install into ${CMAKE_INSTALL_PREFIX}
#     compiz  = Install into compiz prefix (BUILD_GLOBAL=true)
#     local   = Install into home directory
#
# COMPIZ_I18N_DIR      = Translation file directory
#
# COMPIZ_DISABLE_SCHEMAS_INSTALL  = Disables gconf schema installation with gconftool
# COMPIZ_INSTALL_GCONF_SCHEMA_DIR = Installation path of the gconf schema file
#
# VERSION = package version that is added to a plugin pkg-version file
#
#######################################################################

include (CompizCommon)
include (CompizBcop)

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

### Set up core lib dependences so this in correctly imported into plugins

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
    "libstartup-notification-1.0 >= 0.7"
)

compiz_pkg_check_modules (COMPIZ REQUIRED ${COMPIZ_REQUIRES})
list (APPEND COMPIZ_LIBRARIES ${Boost_LIBRARIES})

# determinate installation directories
macro (_prepare_directories)
    set (CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRe" CACHE INTERNAL "" FORCE)
    if ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "package")
	set (PLUGIN_BUILDTYPE global)
	set (PLUGIN_PREFIX    ${CMAKE_INSTALL_PREFIX})
	set (PLUGIN_LIBDIR    ${libdir}/compiz)
	set (PLUGIN_INCDIR    ${includedir})
	set (PLUGIN_PKGDIR    ${libdir}/pkgconfig)
	set (PLUGIN_XMLDIR    ${datadir}/compiz)

	if (NOT CMAKE_BUILD_TYPE)
	     set (CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type (Debug/Release/RelWithDebInfo/MinSizeRe)" FORCE)
	endif (NOT CMAKE_BUILD_TYPE)
    elseif ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "compiz" OR
	    "$ENV{BUILD_GLOBAL}" STREQUAL "true")
	set (PLUGIN_BUILDTYPE global)
	set (PLUGIN_PREFIX    ${COMPIZ_PREFIX})
	set (PLUGIN_LIBDIR    ${COMPIZ_LIBDIR}/compiz)
	set (PLUGIN_INCDIR    ${COMPIZ_INCLUDEDIR})
	set (PLUGIN_PKGDIR    ${COMPIZ_LIBDIR}/pkgconfig)
	set (PLUGIN_XMLDIR    ${COMPIZ_PREFIX}/share/compiz)

	if (NOT CMAKE_BUILD_TYPE)
	     set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug/Release/RelWithDebInfo/MinSizeRe)" FORCE)
	endif (NOT CMAKE_BUILD_TYPE)
    else ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "compiz" OR
	  "$ENV{BUILD_GLOBAL}" STREQUAL "true")
	set (PLUGIN_BUILDTYPE local)
	set (PLUGIN_PREFIX    $ENV{HOME}/.compiz-1)
	set (PLUGIN_LIBDIR    $ENV{HOME}/.compiz-1/plugins)
	set (PLUGIN_XMLDIR    $ENV{HOME}/.compiz-1/metadata)

	if (NOT CMAKE_BUILD_TYPE)
	     set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug/Release/RelWithDebInfo/MinSizeRe)" FORCE)
	endif (NOT CMAKE_BUILD_TYPE)
    endif ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "package")
endmacro (_prepare_directories)

# check plugin dependencies
macro (_check_plugin_plugin_deps _prefix)
    set (${_prefix}_HAS_PLUGIN_DEPS TRUE)
    foreach (_val ${ARGN})
	string (TOUPPER ${_val} _name)

	find_file (
	    _plugin_dep_${_val}
	    compiz-${_val}.pc.in
	    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../${_val}
	    NO_DEFAULT_PATH
	)

	if (_plugin_dep_${_val})
	    file (RELATIVE_PATH _relative ${CMAKE_CURRENT_SOURCE_DIR} ${_plugin_dep_${_val}})
	    get_filename_component (_plugin_inc_dir ${_relative} PATH)
	    list (APPEND ${_prefix}_LOCAL_INCDIRS "${_plugin_inc_dir}/include")
	    list (APPEND ${_prefix}_LOCAL_LIBRARIES ${_val})
	else ()
	    # fallback to pkgconfig
	    compiz_pkg_check_modules (_${_name} compiz-${_val})
	    if (_${_name}_FOUND)
		list (APPEND ${_prefix}_PKG_LIBDIRS "${_${_name}_LIBRARY_DIRS}")
		list (APPEND ${_prefix}_PKG_LIBRARIES "${_${_name}_LIBRARIES}")
	        list (APPEND ${_prefix}_PKG_INCDIRS "${_${_name}_INCLUDE_DIRS}")
	    else ()
		set (${_prefix}_HAS_PLUGIN_DEPS FALSE)
		compiz_set (COMPIZ_${_prefix}_MISSING_DEPS "${COMPIZ_${_prefix}_MISSING_DEPS} compiz-${_val}")
	    endif ()
	endif ()

	compiz_set (_plugin_dep_${_val} "${_plugin_dep_${_val}}")

    endforeach ()
endmacro ()

# main function
macro (_build_compiz_plugin plugin)
    string (TOUPPER ${plugin} _PLUGIN)

    if (COMPIZ_PLUGIN_INSTALL_TYPE)
	set (
	    COMPIZ_PLUGIN_INSTALL_TYPE ${COMPIZ_PLUGIN_INSTALL_TYPE} CACHE STRING
	    "Where a plugin should be installed \(package | compiz | local\)"
	)
    else (COMPIZ_PLUGIN_INSTALL_TYPE)
	set (
	    COMPIZ_PLUGIN_INSTALL_TYPE "local" CACHE STRING
	    "Where a plugin should be installed \(package | compiz | local\)"
	)
    endif (COMPIZ_PLUGIN_INSTALL_TYPE)

    _get_parameters (${_PLUGIN} ${ARGN})
    _prepare_directories ()

    find_file (
	_${plugin}_xml_in ${plugin}.xml.in
	PATHS ${CMAKE_CURRENT_SOURCE_DIR}
	NO_DEFAULT_PATH
    )
    if (_${plugin}_xml_in)
	set (_${plugin}_xml ${_${plugin}_xml_in})
    else ()
	find_file (
	    _${plugin}_xml ${plugin}.xml
	    PATHS ${CMAKE_CURRENT_SOURCE_DIR} }
	    NO_DEFAULT_PATH
        )
    endif ()

    set (${_PLUGIN}_HAS_PKG_DEPS)
    set (${_PLUGIN}_HAS_PLUGIN_DEPS)

    # check dependencies
    compiz_unset (COMPIZ_${_PLUGIN}_MISSING_DEPS)
    _check_plugin_plugin_deps (${_PLUGIN} ${${_PLUGIN}_PLUGINDEPS})
    _check_pkg_deps (${_PLUGIN} ${${_PLUGIN}_PKGDEPS})

    if (${_PLUGIN}_HAS_PKG_DEPS AND ${_PLUGIN}_HAS_PLUGIN_DEPS)

	compiz_set (COMPIZ_${_PLUGIN}_BUILD TRUE PARENT_SCOPE)

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/generated)
	    file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)
	endif (NOT EXISTS ${CMAKE_BINARY_DIR}/generated)

	if (_${plugin}_xml_in)
	    # translate xml
	    compiz_translate_xml ( ${_${plugin}_xml_in} "${CMAKE_BINARY_DIR}/generated/${plugin}.xml")
	    set (_translation_sources "${CMAKE_BINARY_DIR}/generated/${plugin}.xml")
	    set (_translated_xml ${CMAKE_BINARY_DIR}/generated/${plugin}.xml)
	else ()
	    if (_${plugin}_xml)
		set (_translated_xml ${_${plugin}_xml})
	    endif ()
	endif ()

	if (_${plugin}_xml)
	    # do we need bcop for our plugin
	    compiz_plugin_needs_bcop (${_${plugin}_xml} _needs_bcop)
	    if (_needs_bcop)
		# initialize everything we need for bcop
		compiz_add_bcop_targets (${plugin} ${_${plugin}_xml} _bcop_sources)
	    endif ()
	endif ()

	if (_translated_xml)

	    # install xml
	    install (
		FILES ${_translated_xml}
		DESTINATION $ENV{DESTDIR}${PLUGIN_XMLDIR}
	    )
	endif (_translated_xml)

	find_file (
	    _${plugin}_pkg compiz-${plugin}.pc.in
	    PATHS ${CMAKE_CURRENT_SOURCE_DIR}
	    NO_DEFAULT_PATH
	)

	set (COMPIZ_CURRENT_PLUGIN ${plugin})
	set (COMPIZ_CURRENT_XML_FILE ${_translated_xml})

	# find extension files
	file (GLOB _extension_files "${COMPIZ_CMAKE_MODULE_PATH}/plugin_extensions/*.cmake")

	foreach (_file ${_extension_files})
	    include (${_file})
	endforeach ()

	# generate pkgconfig file and install it and the plugin header file
	if (_${plugin}_pkg AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/${plugin})
	    if ("${PLUGIN_BUILDTYPE}" STREQUAL "local")
		message (STATUS "[WARNING] The plugin ${plugin} might be needed by other plugins. Install it systemwide.")
	    else ()
		set (prefix ${PLUGIN_PREFIX})
		set (libdir ${PLUGIN_LIBDIR})
		set (includedir ${PLUGIN_INCDIR})
		if (NOT VERSION)
		    set (VERSION 0.0.1-git)
		endif (NOT VERSION)

		#add CFLAGSADD so pkg-config file has correct flags
		set (COMPIZ_CFLAGS ${COMPIZ_CFLAGS} ${${_PLUGIN}_CFLAGSADD})

		compiz_configure_file (
		    ${_${plugin}_pkg}
		    ${CMAKE_BINARY_DIR}/generated/compiz-${plugin}.pc
		    COMPIZ_REQUIRES
		    COMPIZ_CFLAGS
		    PKGCONFIG_LIBS
		)

		install (
		    FILES ${CMAKE_BINARY_DIR}/generated/compiz-${plugin}.pc
		    DESTINATION ${PLUGIN_PKGDIR}
		)
		install (
		    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${plugin}
		    DESTINATION ${PLUGIN_INCDIR}/compiz
		)
	    endif ()
	endif ()

	# find files for build
	file (GLOB _h_files "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
	file (GLOB _h_ins_files "${CMAKE_CURRENT_SOURCE_DIR}/include/${plugin}/*.h")
	file (GLOB _cpp_files "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

#	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs")

#	set (_cflags "-Wall -Wpointer-arith  -fno-strict-aliasing")


	add_definitions (-DPREFIX='\"${PLUGIN_PREFIX}\"'
			 ${COMPIZ_DEFINITIONS_ADD})

	foreach (_def ${_PLUGIN}_DEFSADD)
	    add_definitions (-D${_def})
	endforeach (_def)

	# Need to know the include-dirs for the internal
	# modules to this plugin, core (if built with core)
	# and any other plugins that we depend on

	get_property (${_PLUGIN}_MOD_INCLUDE_DIRS
		      GLOBAL
		      PROPERTY ${_PLUGIN}_MOD_INCLUDE_DIRS)

	get_property (CORE_MOD_INCLUDE_DIRS
		      GLOBAL
		      PROPERTY CORE_MOD_INCLUDE_DIRS)

	foreach (_plugindep ${${_PLUGIN}_PLUGINDEPS})
	    string (TOUPPER ${_plugindep} _PLUGINDEP)
	    get_property (${_PLUGINDEP}_MOD_INCLUDE_DIRS
			  GLOBAL
			  PROPERTY ${_PLUGINDEP}_MOD_INCLUDE_DIRS)
	    list (APPEND ${_PLUGIN}_PLUGINDEP_MOD_INCLUDE_DIRS ${${_PLUGINDEP}_MOD_INCLUDE_DIRS})
	endforeach (_plugindep)

	include_directories (
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_BINARY_DIR}/generated
            ${${_PLUGIN}_LOCAL_INCDIRS}
            ${${_PLUGIN}_PKG_INCDIRS}
            ${${_PLUGIN}_INCDIRS}
            ${COMPIZ_INCLUDE_DIRS}
            ${PLUGIN_PREFIX}/include
            ${CMAKE_INCLUDE_PATH}
	    ${${_PLUGIN}_MOD_INCLUDE_DIRS}
	    ${CORE_MOD_INCLUDE_DIRS}
	    ${${_PLUGIN}_PLUGINDEP_MOD_INCLUDE_DIRS}
	)

	get_property (${_PLUGIN}_MOD_LIBRARY_DIRS
		      GLOBAL
		      PROPERTY ${_PLUGIN}_MOD_LIBRARY_DIRS)

	set (SYSTEM_LINK_DIRS
	     "/usr/lib"
	     "/usr/lib32"
	     "/usr/lib64"
	     "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")

	link_directories (
            ${COMPIZ_LINK_DIRS}
            ${${_PLUGIN}_PKG_LIBDIRS}
            ${${_PLUGIN}_LIBDIRS}
            ${COMPIZ_LIBDIR}
            ${PLUGIN_LIBDIR}
	    ${${_PLUGIN}_MOD_LIBRARY_DIRS}
            ${SYSTEM_LINK_DIRS}
	)

	add_library (
            ${plugin} SHARED ${_cpp_files}
			     ${_h_files}
			     ${_h_ins_files}
			     ${_bcop_sources}
			     ${_translation_sources}
			     ${COMPIZ_CURRENT_SOURCES_ADDS}
	)

	if (COMPIZ_BUILD_WITH_RPATH)
	    set_target_properties (
		${plugin} PROPERTIES
		INSTALL_RPATH_USE_LINK_PATH 1
        	BUILD_WITH_INSTALL_RPATH 1
        	SKIP_BUILD_RPATH 0
		COMPILE_FLAGS "${${_PLUGIN}_CFLAGSADD}"
		LINK_FLAGS "${${_PLUGIN}_LDFLAGSADD}"
	    )
	else (COMPIZ_BUILD_WITH_RPATH)
	    set_target_properties (
		${plugin} PROPERTIES
		INSTALL_RPATH_USE_LINK_PATH 0
		SKIP_BUILD_RPATH 1
		INSTALL_RPATH "${COMPIZ_LIBDIR}/compiz"
		COMPILE_FLAGS "${${_PLUGIN}_CFLAGSADD}"
		LINK_FLAGS "${${_PLUGIN}_LDFLAGSADD}"
	    )
	endif (COMPIZ_BUILD_WITH_RPATH)

	get_property (${_PLUGIN}_MOD_LIBRARIES
		      GLOBAL
		      PROPERTY ${_PLUGIN}_MOD_LIBRARIES)

	target_link_libraries (
	    ${plugin} ${COMPIZ_LIBRARIES}
		      ${${_PLUGIN}_LOCAL_LIBRARIES}
		      ${${_PLUGIN}_PKG_LIBRARIES}
		      ${${_PLUGIN}_LIBRARIES}
		      ${${_PLUGIN}_MOD_LIBRARIES}
		      compiz_core
	)

	install (
	    TARGETS ${plugin}
	    LIBRARY DESTINATION ${PLUGIN_LIBDIR}
	)

	compiz_add_uninstall ()

	if (NOT COMPIZ_PLUGIN_PACK_BUILD)
		set (CMAKE_PROJECT_NAME plugin-${plugin})

		if (EXISTS ${CMAKE_SOURCE_DIR}/VERSION)
			file (READ ${CMAKE_SOURCE_DIR}/VERSION COMPIZ_RELEASE_VERSION LIMIT 12 OFFSET 0)
		endif (EXISTS ${CMAKE_SOURCE_DIR}/VERSION)

		if (NOT COMPIZ_RELEASE_VERSION)
			set (COMPIZ_RELEASE_VERSION "0.0.1-git")
		endif (NOT COMPIZ_RELEASE_VERSION)

		string (STRIP ${COMPIZ_RELEASE_VERSION} COMPIZ_RELEASE_VERSION)

		set (VERSION ${COMPIZ_RELEASE_VERSION})

		compiz_add_git_dist ()
		compiz_add_release ()
		compiz_add_distcheck ()
		compiz_add_release_signoff ()
	endif (NOT COMPIZ_PLUGIN_PACK_BUILD)

    else ()
	message (STATUS "[WARNING] One or more dependencies for compiz plugin ${plugin} not found. Skipping plugin.")
	message (STATUS "Missing dependencies :${COMPIZ_${_PLUGIN}_MISSING_DEPS}")
	compiz_set (COMPIZ_${_PLUGIN}_BUILD FALSE)
    endif ()
endmacro ()

macro (compiz_plugin plugin)
    string (TOUPPER ${plugin} _PLUGIN)

    # If already defined, use the existing value...
    if (NOT DEFINED COMPIZ_DISABLE_PLUGIN_${_PLUGIN})
       set (COMPIZ_DISABLE_PLUGIN_${_PLUGIN} OFF)
    endif ()

    option (
	COMPIZ_DISABLE_PLUGIN_${_PLUGIN}
	"Disable building of plugin \"${plugin}\""
	${COMPIZ_DISABLE_PLUGIN_${_PLUGIN}}
    )

    if (NOT COMPIZ_DISABLE_PLUGIN_${_PLUGIN})
	_build_compiz_plugin (${plugin} ${ARGN})
    endif (NOT COMPIZ_DISABLE_PLUGIN_${_PLUGIN})
endmacro ()
