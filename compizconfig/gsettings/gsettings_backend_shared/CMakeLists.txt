include (FindPkgConfig)

compiz_pkg_check_modules (GSETTINGS_UTIL ${_deps})

if (GSETTINGS_UTIL_FOUND)

    set (GSETTINGS_UTIL_LIB_INCLUDES
         ${CMAKE_SOURCE_DIR}/compizconfig/integration/gnome/include
         ${CMAKE_SOURCE_DIR}/compizconfig/integration/gnome/gconf/include
         ${CMAKE_SOURCE_DIR}/compizconfig/integration/gnome/gsettings/include
	 ${compiz_SOURCE_DIR}/compizconfig/libcompizconfig/include
	 ${GSETTINGS_UTIL_INCLUDE_DIRS}
	 ${CMAKE_CURRENT_SOURCE_DIR}/../src)

    include_directories (${GSETTINGS_UTIL_LIB_INCLUDES})

    link_directories (${GSETTINGS_UTIL_LIBRARY_DIRS}
		      ${compiz_BINARY_DIR}/compizconfig/libcompizconfig
                      ${compiz_BINARY_DIR}/compizconfig/integration/gnome
                      ${CMAKE_BINARY_DIR}/compizconfig/integration/gnome/gconf
                      ${CMAKE_BINARY_DIR}/compizconfig/integration/gnome/gsettings)

    add_library (compizconfig_gsettings_backend_interface STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_backend_interface.c)

    add_library (compizconfig_gsettings_wrapper_interface STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_interface.c)

    add_library (compizconfig_gsettings_wrapper STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_interface_wrapper.c)

    target_link_libraries (compizconfig_gsettings_wrapper
                           compizconfig
                           ${GSETTINGS_UTIL_LIBRARIES})

    add_library (compizconfig_gsettings_util STATIC
                 ${CMAKE_CURRENT_SOURCE_DIR}/gsettings_constants.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/gsettings_util.c)

    target_link_libraries (compizconfig_gsettings_util
                           compizconfig_gsettings_backend_interface
                           compizconfig_gsettings_wrapper_interface
                           compizconfig
                           ${GSETTINGS_UTIL_LIBRARIES})

    add_library (compizconfig_gsettings_backend SHARED
                 ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_backend.c)

    set (GSETTINGS_BACKEND_SHARED_ADDITIONAL_LIBRARIES
	 compizconfig_gsettings_backend_interface
	 compizconfig_gsettings_util
	 compizconfig_gsettings_wrapper
	 compizconfig_gnome_integration
	 compizconfig_gnome_integration_gsettings_setting_factory
	 compizconfig)

    if (USE_GCONF)
	list (APPEND GSETTINGS_BACKEND_SHARED_ADDITIONAL_LIBRARIES
	      compizconfig_gnome_integration_gconf_setting_factory)
	list (APPEND GSETTINGS_BACKEND_SHARED_ADDITIONAL_LIBRARIES
	      compizconfig_gnome_integration_gconf_integrated_setting)
    endif (USE_GCONF)

    target_link_libraries (compizconfig_gsettings_backend
			   ${GSETTINGS_BACKEND_SHARED_ADDITIONAL_LIBRARIES}
                           ${GSETTINGS_UTIL_LIBRARIES})

    install (TARGETS compizconfig_gsettings_backend
	     DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

    #
    # Tell CMake that targets using compizconfig_gsettings_backend should NOT re-import the
    # libraries that compizconfig_gsettings_backend depends on (contains).
    #
    set_target_properties (compizconfig_gsettings_backend PROPERTIES
	LINK_INTERFACE_LIBRARIES ""
    )

endif (GSETTINGS_UTIL_FOUND)
