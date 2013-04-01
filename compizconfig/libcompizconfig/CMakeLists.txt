project (libcompizconfig)

set (COMPIZ_CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${COMPIZ_CMAKE_MODULE_PATH})
include (CompizDefaults)
include (CompizCommon)
include (CompizPackage)
include (CheckLibraryExists)
include (CheckIncludeFile)

set (CMAKE_MODULE_PATH_ORIG ${CMAKE_MODULE_PATH})
set (LIBCOMPIZCONFIG_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LIBCOMPIZCONFIG_CMAKE_MODULE_PATH})

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set (ALL_LINGUAS af ar bg bn bn_IN bs ca cs cy da de el en_GB en_US es eu et fi fr gl gu he hi hr hu id it ja ka km ko lo lt mk mr nb nl or pa pl pt pt_BR ro ru sk sl sr sv ta tr uk vi xh zh_CN zh_TW zu)
set (GETTEXT_PACKAGE libcompizconfig)

file (READ ${CMAKE_SOURCE_DIR}/VERSION COMPIZ_RELEASE_VERSION LIMIT 12 OFFSET 0)
string (STRIP ${COMPIZ_RELEASE_VERSION} COMPIZ_RELEASE_VERSION)
set (VERSION ${COMPIZ_RELEASE_VERSION})

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

include_directories (
    "include"
    )

if (COMPIZ_PACKAGING_ENABLED)
    set (libcompizconfig_libdir ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}         CACHE PATH "libdir")
    set (libcompizconfig_includedir ${CMAKE_INSTALL_PREFIX}/include/compizconfig CACHE PATH "includedir")
    set (libcompizconfig_prefix ${CMAKE_INSTALL_PREFIX}                          CACHE PATH "prefix")
    set (libcompizconfig_exec_prefix ${CMAKE_INSTALL_PREFIX}/bin                 CACHE PATH "bindir")
else (COMPIZ_PACKAGING_ENABLED)
    set (libcompizconfig_libdir ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
    set (libcompizconfig_includedir ${CMAKE_INSTALL_PREFIX}/include/compizconfig)
    set (libcompizconfig_prefix ${CMAKE_INSTALL_PREFIX})
    set (libcompizconfig_exec_prefix ${CMAKE_INSTALL_PREFIX}/bin)
endif (COMPIZ_PACKAGING_ENABLED)

set (COMPIZ_SYSCONFDIR "/etc" CACHE PATH "/etc")

option (USE_PROTOBUF
	"Use Google Protocol Buffers library for faster XML loading"
	ON
	)

set (PROTOC_ERROR_MESSAGE
     "\nWARNING: You need to install Protocol Buffers to get faster program startup.\n"
     "Sources are available at http://code.google.com/p/protobuf/downloads/list\n"
     "Package names for Ubuntu/Debian: libprotobuf0, libprotobuf-dev, protobuf-compiler\n"
     "Package names for other distributions: protobuf, protobuf-devel\n"
     "Disabling protobuf.\n"
     )

set (LIBCOMPIZCONFIG_PROTOBUF
     protobuf)

if (USE_PROTOBUF)
    find_program (HAVE_PROTOC protoc)
    if (HAVE_PROTOC)
	if (USE_PROTOBUF)
	    pkg_check_modules (PROTOBUF protobuf)
	    if (PROTOBUF_FOUND)
	    else ()
		message (${PROTOC_ERROR_MESSAGE})
		set (USE_PROTOBUF
		     false
		     )
	    endif (PROTOBUF_FOUND)
	endif (USE_PROTOBUF)
    else ()
	message (${PROTOC_ERROR_MESSAGE})
	set (USE_PROTOBUF
	     false
	     )
    endif (HAVE_PROTOC)
endif (USE_PROTOBUF)

if (USE_PROTOBUF)
    set (HAVE_PROTOBUF TRUE)
endif (USE_PROTOBUF)

check_include_file ("sys/inotify.h"
		    HAVE_INOTIFY)

set (INOTIFY_WARNING_MESSAGE
     "NOTE\n
      ======\n
      Inotify header files not found. While this library might work \n
      without them, user experience is degraded as settings won't be\n
      applied instantly. We strongly suggest installing the inotify\n
      header files.\n")

if (HAVE_INOTIFY)
else ()
    message (${INOTIFY_WARNING_MESSAGE})
endif (HAVE_INOTIFY)

set (COMPIZCONFIG_LIBDIR ${libdir})

set (LIBCOMPIZCONFIG_REQUIRES
    x11
    libxml-2.0
    libxslt
    )

if (HAVE_PROTOBUF)
    set (LIBCOMPIZCONFIG_REQUIRES
	 ${LIBCOMPIZCONFIG_REQUIRES}
	 protobuf)
endif ()

compiz_pkg_check_modules (LIBCOMPIZCONFIG REQUIRED ${LIBCOMPIZCONFIG_REQUIRES})

list (APPEND LIBCOMPIZCONFIG_LIBRARIES ${COMPIZ_LIBRARIES})
list (APPEND LIBCOMPIZCONFIG_LIBRARY_DIRS ${COMPIZ_LIBRARY_DIRS})
list (APPEND LIBCOMPIZCONFIG_INCLUDE_DIRS ${COMPIZ_INCLUDE_DIRS})

compiz_configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/libcompizconfig.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libcompizconfig.pc
)

compiz_configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/libcompizconfig_internal.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libcompizconfig_internal.pc
)

install (
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libcompizconfig.pc
    DESTINATION ${libdir}/pkgconfig
)


add_subdirectory (config)
add_subdirectory (backend)
add_subdirectory (src)
add_subdirectory (include)
add_subdirectory (cmake)

if (COMPIZ_BUILD_TESTING)
    add_subdirectory (tests)
endif (COMPIZ_BUILD_TESTING)
