if (USE_GTK)

    if (COMPIZ_BUILD_WITH_RPATH)
	set (CMAKE_INSTALL_RPATH ${libdir})
    endif (COMPIZ_BUILD_WITH_RPATH)

    include_directories (
	${compiz_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/gtk
	${GTK_WINDOW_DECORATOR_INCLUDE_DIRS}
	${METACITY_INCLUDE_DIRS}
	${GCONF_INCLUDE_DIRS}
	${DBUS_GLIB_INCLUDE_DIRS}
    )

    add_definitions (
	-DHAVE_CONFIG_H
	-DALL_LINGUAS=\"${ALL_LINGUAS}\"
	-DLOCALEDIR=\\\"${datadir}/locale\\\"
    )

    link_directories (
	${COMPIZ_LINK_DIRS}
    )

    add_library (gtk_window_decorator_metacity_window_decoration_util
		 ${CMAKE_CURRENT_SOURCE_DIR}/gwd-metacity-window-decoration-util.c)

    add_library (gtk_window_decorator_cairo_window_decoration_util
		 ${CMAKE_CURRENT_SOURCE_DIR}/gwd-cairo-window-decoration-util.c)

    add_library (gtk_window_decorator_settings_interface STATIC
		 ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings-interface.c)

    add_library (gtk_window_decorator_settings STATIC
		 ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings.c)

    add_library (gtk_window_decorator_settings_storage_interface STATIC
		 ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings-storage-interface.c)

    add_library (gtk_window_decorator_settings_storage_xproperty_interface STATIC
		 ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings-xproperty-interface.c)

    add_library (gtk_window_decorator_settings_writable_interface STATIC
		 ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings-writable-interface.c)

    add_library (gtk_window_decorator_settings_notified_interface
		 ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings-notified-interface.c)

    set (GTK_WINDOW_DECORATOR_ADDITIONAL_LIBRARIES
	 gtk_window_decorator_settings_notified_interface
	 gtk_window_decorator_settings_interface
	 gtk_window_decorator_settings_writable_interface
	 gtk_window_decorator_settings
	 gtk_window_decorator_settings_storage_interface
	 gtk_window_decorator_settings_storage_xproperty_interface
	 gtk_window_decorator_metacity_window_decoration_util
	 gtk_window_decorator_cairo_window_decoration_util)

    if (USE_GCONF)
	set (gwd_schema ${CMAKE_CURRENT_BINARY_DIR}/gwd.schemas)
	compiz_translate_xml (
	    ${CMAKE_CURRENT_SOURCE_DIR}/gwd.schemas.in
	    ${gwd_schema}
	)
	compiz_install_gconf_schema (${CMAKE_CURRENT_BINARY_DIR}/gwd.schemas ${SCHEMADIR})
	add_library (gtk_window_decorator_settings_storage_gconf STATIC
		     ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings-storage-gconf.c)
	list (APPEND GTK_WINDOW_DECORATOR_ADDITIONAL_LIBRARIES
	      gtk_window_decorator_settings_storage_gconf)
    endif (USE_GCONF)

    if (USE_GSETTINGS)
	add_definitions (-DUSE_GSETTINGS)
	add_library (gtk_window_decorator_settings_storage_gsettings STATIC
		     ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings-storage-gsettings.c)

	compiz_pkg_check_modules (GSETTINGS gio-2.0 glib-2.0)

	target_link_libraries (gtk_window_decorator_settings_storage_gsettings
			       ${GSETTINGS_LIBRARIES})

	list (APPEND GTK_WINDOW_DECORATOR_ADDITIONAL_LIBRARIES
	      gtk_window_decorator_settings_storage_gsettings)

	set (_gwd_gschema_name org.compiz.gwd)
	set (_gwd_gschema_filename ${_gwd_gschema_name}.gschema.xml)
	set (_gwd_gschema_filepath ${CMAKE_CURRENT_SOURCE_DIR}/${_gwd_gschema_filename})
	set (_gwd_gschema_generated_location ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/${_gwd_gschema_filename})
	set (_gwd_gschema_dest ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/)

	add_custom_command (OUTPUT ${_gwd_gschema_generated_location}
			    COMMAND cp -r ${_gwd_gschema_filepath} ${_gwd_gschema_generated_location}
			    DEPENDS ${_gwd_gschema_filepath}
			    VERBATIM)

	add_custom_target (compiz_gwd_gsettings_org_compiz_gwd_schema ALL
			   DEPENDS ${_gwd_gschema_generated_location})

	add_gsettings_schema_to_recompilation_list (compiz_gwd_gsettings_org_compiz_gwd_schema)

	compiz_install_gsettings_schema (${_gwd_gschema_generated_location}
					 ${_gwd_gschema_dest})
    endif (USE_GSETTINGS)

    add_executable (gtk-window-decorator
		    gtk-window-decorator.c
		    blurprops.c
		    decorprops.c
		    cairo.c
		    frames.c
		    gdk.c
		    switcher.c
		    metacity.c
		    events.c
		    forcequit.c
		    tooltip.c
		    actionmenu.c
		    settings.c
		    util.c
		    style.c
		    wnck.c
		    decorator.c
		    ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings-xproperty-storage.c
		    ${CMAKE_CURRENT_SOURCE_DIR}/gwd-settings-notified.c
		    ${gwd_schema}
    )

    add_dependencies (gtk-window-decorator decoration)

    if (USE_METACITY)
	set (metacitylibs ${METACITY_LIBRARIES})
    endif (USE_METACITY)


    target_link_libraries (
	gtk-window-decorator
	decoration
	${GTK_WINDOW_DECORATOR_LIBRARIES}
	${GCONF_LIBRARIES}
	${DBUS_GLIB_LIBRARIES}
	${metacitylibs}
	${GTK_WINDOW_DECORATOR_ADDITIONAL_LIBRARIES}
    )

    install (
	TARGETS gtk-window-decorator
	DESTINATION ${exec_prefix}
    )

    if (COMPIZ_BUILD_TESTING)
	add_subdirectory (tests)
    endif (COMPIZ_BUILD_TESTING)

endif (USE_GTK)



