include (${COMPIZ_CMAKE_MODULE_PATH}/CompizGSettings.cmake)

macro (compiz_gsettings_prepare_install_dirs)
    # package
    if ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "package")
	if (NOT COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR)
            set (PLUGIN_SCHEMADIR "${datadir}/glib-2.0/schemas/")
        else (NOT COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR)
	    set (PLUGIN_SCHEMADIR "${COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR}")
	endif (NOT COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR)
    # compiz
    elseif ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "compiz" OR
	    "$ENV{BUILD_GLOBAL}" STREQUAL "true")
	if (NOT COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR)
            set (PLUGIN_SCHEMADIR "${COMPIZ_PREFIX}/share/glib-2.0/schemas/")
        else (NOT COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR)
	    set (PLUGIN_SCHEMADIR "${COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR}")
	endif (NOT COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR)
    # local
    else ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "compiz" OR
	  "$ENV{BUILD_GLOBAL}" STREQUAL "true")

	if (NOT COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR)
            set (PLUGIN_SCHEMADIR "$ENV{HOME}/.config/compiz-1/gsettings/schemas")
        else (NOT COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR)
	    set (PLUGIN_SCHEMADIR "${COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR}")
	endif (NOT COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR)

    endif ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "package")
endmacro (compiz_gsettings_prepare_install_dirs)

if (USE_GSETTINGS)

    compiz_gsettings_prepare_install_dirs ()

    set (PLUGIN_GSETTINGS_SCHEMA_DST
	 ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/org.compiz.${COMPIZ_CURRENT_PLUGIN}.gschema.xml)

    compiz_gsettings_schema (${COMPIZ_CURRENT_PLUGIN}
			     ${COMPIZ_CURRENT_XML_FILE}
			     ${PLUGIN_GSETTINGS_SCHEMA_DST}
			     ${PLUGIN_SCHEMADIR})
    list (APPEND COMPIZ_CURRENT_SOURCES_ADDS ${PLUGIN_GSETTINGS_SCHEMA_DST})

endif (USE_GSETTINGS)
