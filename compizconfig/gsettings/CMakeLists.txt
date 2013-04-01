project (compizconfig-backend-gsettings)

find_package (CompizConfig REQUIRED)

include (LibCompizConfigCommon)
include (CompizGSettings)

set (_deps glib-2.0>=2.20.0
	   gio-2.0>=2.25.0)

if (USE_GCONF)
    list (APPEND _deps gconf-2.0>=2.31.1)

    add_definitions (-DUSE_GCONF)
endif (USE_GCONF)

if (COMPIZ_BUILD_TESTING)
    add_subdirectory (mocks)
endif (COMPIZ_BUILD_TESTING)

add_subdirectory (gsettings_backend_shared)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/gsettings_backend_shared
                     ${CMAKE_SOURCE_DIR}/compizconfig/integration/gnome/gsettings/include)

compizconfig_backend (gsettings PKGDEPS ${_deps} LIBRARIES compizconfig_gsettings_backend)

find_program (GLIB_COMPILE_SCHEMAS glib-compile-schemas)

if (GLIB_COMPILE_SCHEMAS)

    set (_top_gschema      org.compiz)
    set (_top_gschema_file ${_top_gschema}.gschema.xml)
    set (_top_gschema_src  ${CMAKE_CURRENT_SOURCE_DIR}/${_top_gschema_file})
    set (_top_gschema_dest ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/${_top_gschema_file})
    set (_top_gschema_inst ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/)

    configure_file (${_top_gschema_src} ${_top_gschema_dest})

    add_custom_command (
	OUTPUT ${CMAKE_BINARY_DIR}/glib-2.0/schemas/gschemas.compiled
	COMMAND ${GLIB_COMPILE_SCHEMAS} --targetdir=${CMAKE_BINARY_DIR}/glib-2.0/schemas/
		${CMAKE_BINARY_DIR}/glib-2.0/schemas/
    )

    add_custom_command (OUTPUT ${_top_gschema_dest}
			COMMAND cp -r ${_top_gschema_src} ${_top_gschema_dest}
			DEPENDS ${_top_gschema_src}
			VERBATIM)

    add_custom_target (compiz_gsettings_schema
		       DEPENDS ${_top_gschema_dest})

    add_gsettings_schema_to_recompilation_list (compiz_gsettings_schema)

    compiz_install_gsettings_schema (${_top_gschema_dest}
				     ${_top_gschema_inst})

endif (GLIB_COMPILE_SCHEMAS)

if (COMPIZ_BUILD_TESTING)
    add_subdirectory (tests)
endif (COMPIZ_BUILD_TESTING)
