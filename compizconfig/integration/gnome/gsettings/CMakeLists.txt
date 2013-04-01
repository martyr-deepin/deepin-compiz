include (FindPkgConfig)

if (USE_GSETTINGS)

    set (_deps
	glib-2.0>=2.20.0
	gio-2.0>=2.25.0)

    compiz_pkg_check_modules (COMPIZCONFIG_GNOME_GSETTINGS_INTEGRATION ${_deps})

    if (COMPIZCONFIG_GNOME_GSETTINGS_INTEGRATION_FOUND)

	add_definitions (-DUSE_GCONF)

	include_directories (${CMAKE_SOURCE_DIR}/compizconfig/libcompizconfig/include
			     ${CMAKE_SOURCE_DIR}/compizconfig/gsettings/gsettings_backend_shared
			     ${CMAKE_SOURCE_DIR}/compizconfig/integration/gnome/include
			     ${CMAKE_SOURCE_DIR}/compizconfig/integration/gnome/gsettings/include
			     ${COMPIZCONFIG_GNOME_GSETTINGS_INTEGRATION_INCLUDE_DIRS})

	link_directories (${CMAKE_BINARY_DIR}/compizconfig/libcompizconfig
			  ${CMAKE_BINARY_DIR}/compizconfig/gsettings/gsettings_backend_shared
			  ${CMAKE_BINARY_DIR}/compizconfig/integration/gnome
			  ${COMPIZCONFIG_GNOME_GSETTINGS_INTEGRATION_LIBRARY_DIRS})

	add_library (compizconfig_gnome_integration_gsettings_integrated_setting STATIC
		     ${CMAKE_CURRENT_SOURCE_DIR}/src/ccs_gnome_integration_gsettings_integrated_setting.c)

	target_link_libraries (compizconfig_gnome_integration_gsettings_integrated_setting
			       compizconfig_gnome_integrated_setting
			       compizconfig_gnome_integration_constants
			       compizconfig
                               compizconfig_gsettings_util
			       ${COMPIZCONFIG_GNOME_GSETTINGS_INTEGRATION_LIBRARIES})

	add_library (compizconfig_gnome_integration_gsettings_setting_factory STATIC
		     ${CMAKE_CURRENT_SOURCE_DIR}/src/ccs_gnome_integration_gsettings_integrated_setting_factory.c)

	target_link_libraries (compizconfig_gnome_integration_gsettings_setting_factory
			       compizconfig_gnome_integration_constants
			       compizconfig_gnome_integrated_setting
			       compizconfig_gnome_integration_gsettings_integrated_setting
			       compizconfig
                               compizconfig_gsettings_util
			       ${COMPIZCONFIG_GNOME_GSETTINGS_INTEGRATIONLIBRARIES})

	find_program (GLIB_COMPILE_SCHEMAS glib-compile-schemas)

	if (GLIB_COMPILE_SCHEMAS)

	    set (_integrated_gschema      org.compiz.integrated)
	    set (_integrated_gschema_file ${_integrated_gschema}.gschema.xml)
	    set (_integrated_gschema_src  ${CMAKE_CURRENT_SOURCE_DIR}/${_integrated_gschema_file})
	    set (_integrated_gschema_dest ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/${_integrated_gschema_file})

	    configure_file (${_integrated_gschema_src} ${_integrated_gschema_dest})

	    add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/glib-2.0/schemas/gschemas.compiled
		COMMAND ${GLIB_COMPILE_SCHEMAS} --targetdir=${CMAKE_BINARY_DIR}/glib-2.0/schemas/
			${CMAKE_BINARY_DIR}/glib-2.0/schemas/
	    )

	    add_custom_target (compiz_gsettings_integrated_settings_schema ALL
			       DEPENDS ${_integrated_gschema_src})

	    add_gsettings_schema_to_recompilation_list (compiz_gsettings_integrated_settings_schema)

	    compiz_install_gsettings_schema (${_integrated_gschema_dest}
					     ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)

	endif (GLIB_COMPILE_SCHEMAS)

	if (COMPIZ_BUILD_TESTING)
	    add_subdirectory (tests)
	endif (COMPIZ_BUILD_TESTING)

    endif (COMPIZCONFIG_GNOME_GSETTINGS_INTEGRATION_FOUND)

endif (USE_GSETTINGS)
