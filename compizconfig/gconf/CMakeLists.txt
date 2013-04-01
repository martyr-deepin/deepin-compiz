if (USE_GCONF)

    find_package (CompizConfig REQUIRED)

    include (LibCompizConfigCommon)

    include_directories (${CMAKE_SOURCE_DIR}/compizconfig/integration/gnome/include
			 ${CMAKE_SOURCE_DIR}/compizconfig/integration/gnome/gconf/include)
    link_directories (${CMAKE_BINARY_DIR}/compizconfig/integration/gnome
		      ${CMAKE_BINARY_DIR}/compizconfig/integration/gnome/gconf)

    compizconfig_backend (gconf PKGDEPS glib-2.0 gconf-2.0 LIBRARIES
							   compizconfig_gnome_integration
							   compizconfig_gnome_integration_gconf_setting_factory
							   compizconfig_gnome_integration_gconf_integrated_setting)
endif (USE_GCONF)
