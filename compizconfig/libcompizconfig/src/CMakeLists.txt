include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBCOMPIZCONFIG_INCLUDE_DIRS}
)

set (LIBCOMPIZCONFIG_FILES
    bindings.c
    ccs-private.h
    compiz.cpp
    config.c
    filewatch.c
    ini.c
    iniparser.c
    iniparser.h
    lists.c
    main.c
    log.c
)

add_definitions (
    -DHAVE_CONFIG_H
    -DPLUGINDIR=\\\"${COMPIZ_PLUGINDIR}\\\"
    -DIMAGEDIR=\\\"${COMPIZ_IMAGEDIR}\\\"
    -DMETADATADIR=\\\"${COMPIZ_METADATADIR}\\\"
    -DSYSCONFDIR=\\\"${COMPIZ_SYSCONFDIR}\\\"
    -DDATADIR=\\\"${COMPIZ_DATADIR}\\\"
    -DLIBDIR=\\\"${COMPIZ_LIBDIR}\\\"
)

if (HAVE_PROTOBUF)
    add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/compizconfig.pb.cc
			COMMAND ${HAVE_PROTOC} ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR} compizconfig.proto
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set (LIBCOMPIZCONFIG_FILES
	 ${LIBCOMPIZCONFIG_FILES}
	 ${CMAKE_CURRENT_BINARY_DIR}/compizconfig.pb.cc
	)
    set (LIBCOMPIZCONFIG_LIBRARIES
	 ${LIBCOMPIZCONFIG_LIBRARIES}
	 protobuf)
    add_definitions (
	    -DUSE_PROTOBUF
    )
endif (HAVE_PROTOBUF)

if (HAVE_INOTIFY)
    add_definitions (
	    -DHAVE_SYS_INOTIFY_H
    )
endif (HAVE_INOTIFY)

add_library (
    ccs_text_file_interface STATIC
    ccs_text_file_interface.c
)

add_library (
    ccs_text_file STATIC
    ccs_text_file.c
)

target_link_libraries (
    ccs_text_file
    ccs_text_file_interface
)

add_library (
    ccs_settings_upgrade_internal STATIC
    ccs_settings_upgrade_internal.c
)

add_library (
    compizconfig SHARED
    ${LIBCOMPIZCONFIG_FILES}
)

set_target_properties (compizconfig PROPERTIES
    VERSION 0.0.0
    SOVERSION 0
)

link_directories (
    ${LIBCOMPIZCONFIG_LINK_DIRS}
)

target_link_libraries (
    compizconfig
    ${LIBCOMPIZCONFIG_LIBRARIES}
    m
    dl
    ccs_settings_upgrade_internal
    ccs_text_file
)

#
# Tell CMake that targets using compizconfig should NOT re-import the
# libraries that compizconfig depends on (contains).
#
set_target_properties (compizconfig PROPERTIES
		       LINK_INTERFACE_LIBRARIES ""
)

set (COMPIZCONFIG_LIBS "")
foreach (_val ${LIBCOMPIZCONFIG_LDFLAGS})
    set (COMPIZCONFIG_LIBS "${COMPIZCONFIG_LIBS}${_val} ")
endforeach (_val ${LIBCOMPIZCONFIG_LDFLAGS})

set (COMPIZCONFIG_CFLAGS "")
foreach (_val ${LIBCOMPIZCONFIG_CFLAGS})
    set (COMPIZCONFIG_CFLAGS "${COMPIZCONFIG_CFLAGS}${_val} ")
endforeach (_val ${LIBCOMPIZCONFIG_CFLAGS})

install (
    TARGETS compizconfig
    DESTINATION ${libcompizconfig_libdir}
)
