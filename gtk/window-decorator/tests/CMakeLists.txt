include (CompizGSettings)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}
		     ${CMAKE_CURRENT_SOURCE_DIR}/../
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_SOURCE_DIR}/tests/shared
                     ${CMAKE_SOURCE_DIR}/tests/shared/glib)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/compiz_gwd_tests.h.in
		${CMAKE_CURRENT_BINARY_DIR}/compiz_gwd_tests.h)

pkg_check_modules (COMPIZ_TEST_GTK_WINDOW_DECORATOR
		   glib-2.0>=2.28
		   gio-2.0>=2.25.0)

if (COMPIZ_TEST_GTK_WINDOW_DECORATOR_FOUND)

    link_directories (${CMAKE_CURRENT_BINARY_DIR}
		      ${CMAKE_CURRENT_BINARY_DIR}/..)

    add_library (gtk_window_decorator_mock_settings_storage STATIC
		 ${CMAKE_CURRENT_SOURCE_DIR}/compiz_gwd_mock_settings_storage.cpp)

    target_link_libraries (gtk_window_decorator_mock_settings_storage
			   gtk_window_decorator_settings_storage_interface)

    add_library (gtk_window_decorator_mock_settings STATIC
		 ${CMAKE_CURRENT_SOURCE_DIR}/compiz_gwd_mock_settings.cpp)

    target_link_libraries (gtk_window_decorator_mock_settings
			   gtk_window_decorator_settings_interface)

    add_library (gtk_window_decorator_mock_settings_writable STATIC
		 ${CMAKE_CURRENT_SOURCE_DIR}/compiz_gwd_mock_settings_writable.cpp)

    target_link_libraries (gtk_window_decorator_mock_settings_writable
			   gtk_window_decorator_settings_writable_interface)

    add_library (gtk_window_decorator_mock_settings_notified STATIC
		 ${CMAKE_CURRENT_SOURCE_DIR}/compiz_gwd_mock_settings_notified.cpp)

    target_link_libraries (gtk_window_decorator_mock_settings_notified
			   gtk_window_decorator_settings_notified_interface)

    add_executable (compiz_test_gwd_settings
		    ${CMAKE_CURRENT_SOURCE_DIR}/test_gwd_settings.cpp)

    set (COMPIZ_TEST_GWD_SETTINGS_ADDITIONAL_LIBRARIES
	 gtk_window_decorator_settings
	 gtk_window_decorator_mock_settings
	 gtk_window_decorator_mock_settings_writable
	 gtk_window_decorator_mock_settings_storage
	 gtk_window_decorator_mock_settings_notified)

    set (COMPIZ_TEST_GWD_SETTINGS_COVERAGE_TARGETS
	 gtk_window_decorator_settings
	 gtk_window_decorator_settings_interface
	 gtk_window_decorator_settings_storage_interface
	 gtk_window_decorator_settings_notified_interface)

    if (USE_GSETTINGS)
	add_definitions (-DUSE_GSETTINGS)
	set (_desktop_gschema_name org.gnome.desktop.wm.preferences)
	set (_desktop_gschema_filename ${_desktop_gschema_name}.gschema.xml)
	set (_desktop_gschema_filepath ${CMAKE_CURRENT_SOURCE_DIR}/${_desktop_gschema_filename})
	set (_desktop_gschema_generated_location ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/${_desktop_gschema_filename})

	add_custom_command (OUTPUT ${_desktop_gschema_generated_location}
			    COMMAND cp -r ${_desktop_gschema_filepath} ${_desktop_gschema_generated_location}
			    DEPENDS ${_desktop_gschema_filepath}
			    VERBATIM)

	add_custom_target (compiz_gwd_gsettings_org_gnome_desktop_wm_preferences_schema ALL
			   DEPENDS ${_desktop_gschema_generated_location})

	add_gsettings_schema_to_recompilation_list (compiz_gwd_gsettings_org_gnome_desktop_wm_preferences_schema)

	set (_mutter_gschema_name org.gnome.mutter)
	set (_mutter_gschema_filename ${_mutter_gschema_name}.gschema.xml)
	set (_mutter_gschema_filepath ${CMAKE_CURRENT_SOURCE_DIR}/${_mutter_gschema_filename})
	set (_mutter_gschema_generated_location ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/${_mutter_gschema_filename})

	add_custom_command (OUTPUT ${_mutter_gschema_generated_location}
			    COMMAND cp -r ${_mutter_gschema_filepath} ${_mutter_gschema_generated_location}
			    DEPENDS ${_mutter_gschema_filepath}
			    VERBATIM)

	add_custom_target (compiz_gwd_gsettings_org_gnome_mutter_schema ALL
			   DEPENDS ${_mutter_gschema_generated_location})

	add_gsettings_schema_to_recompilation_list (compiz_gwd_gsettings_org_gnome_mutter_schema)

	set (COMPIZ_TEST_GWD_SETTINGS_ADDITIONAL_LIBRARIES
	     ${COMPIZ_TEST_GWD_SETTINGS_ADDITIONAL_LIBRARIES}
	     gtk_window_decorator_settings_storage_gsettings)

	set (COMPIZ_TEST_GWD_SETTINGS_COVERAGE_TARGETS
	     ${COMPIZ_TEST_GWD_SETTINGS_COVERAGE_TARGETS}
	    gtk_window_decorator_settings_storage_gsettings)
    endif (USE_GSETTINGS)


    target_link_libraries (compiz_test_gwd_settings
			   ${COMPIZ_TEST_GWD_SETTINGS_ADDITIONAL_LIBRARIES}
			   decoration
			   ${COMPIZ_TEST_GTK_WINDOW_DECORATOR_LIBRARIES}
			   ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
			   ${GTEST_BOTH_LIBRARIES}
			   ${GMOCK_LIBRARY}
		       	   ${GMOCK_MAIN_LIBRARY})

    compiz_discover_tests (compiz_test_gwd_settings
			   COVERAGE
			   ${COMPIZ_TEST_GWD_SETTINGS_COVERAGE_TARGETS})

    add_executable (compiz_test_gwd_cairo_decorations
		    ${CMAKE_CURRENT_SOURCE_DIR}/test_gwd_cairo_decorations.cpp)

    target_link_libraries (compiz_test_gwd_cairo_decorations
			   gtk_window_decorator_cairo_window_decoration_util
			   ${COMPIZ_TEST_GTK_WINDOW_DECORATOR_LIBRARIES}
			   ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
			   ${GTEST_BOTH_LIBRARIES}
			   ${GMOCK_LIBRARY}
		       	   ${GMOCK_MAIN_LIBRARY}
			   decoration)

    compiz_discover_tests (compiz_test_gwd_cairo_decorations COVERAGE
			   gtk_window_decorator_cairo_window_decoration_util)

    add_executable (compiz_test_gwd_metacity_decorations
		    ${CMAKE_CURRENT_SOURCE_DIR}/test_gwd_metacity_decorations.cpp)

    target_link_libraries (compiz_test_gwd_metacity_decorations
			   gtk_window_decorator_metacity_window_decoration_util
			   ${COMPIZ_TEST_GTK_WINDOW_DECORATOR_LIBRARIES}
			   ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
			   ${GTEST_BOTH_LIBRARIES}
			   ${GMOCK_LIBRARY}
		       	   ${GMOCK_MAIN_LIBRARY}
			   decoration)

    compiz_discover_tests (compiz_test_gwd_metacity_decorations COVERAGE
			   gtk_window_decorator_metacity_window_decoration_util)

endif (COMPIZ_TEST_GTK_WINDOW_DECORATOR_FOUND)
