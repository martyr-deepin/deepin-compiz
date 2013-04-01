option (
    COMPIZ_DISABLE_SCHEMAS_INSTALL
    "Disables gconf schema installation with gconftool"
    OFF
)

set (
    COMPIZ_INSTALL_GCONF_SCHEMA_DIR ${COMPIZ_INSTALL_GCONF_SCHEMA_DIR} CACHE PATH
    "Installation path of the gconf schema file"
)

macro (compiz_gconf_prepare_install_dirs)
    if ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "package")
	if (NOT COMPIZ_INSTALL_GCONF_SCHEMA_DIR)
            set (PLUGIN_SCHEMADIR "${datadir}/gconf/schemas")
        else (NOT COMPIZ_INSTALL_GCONF_SCHEMA_DIR)
	    set (PLUGIN_SCHEMADIR "${COMPIZ_INSTALL_GCONF_SCHEMA_DIR}")
	endif (NOT COMPIZ_INSTALL_GCONF_SCHEMA_DIR)
    elseif ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "compiz" OR
	    "$ENV{BUILD_GLOBAL}" STREQUAL "true")
	if (NOT COMPIZ_INSTALL_GCONF_SCHEMA_DIR)
            set (PLUGIN_SCHEMADIR "${COMPIZ_PREFIX}/share/gconf/schemas")
        else (NOT COMPIZ_INSTALL_GCONF_SCHEMA_DIR)
	    set (PLUGIN_SCHEMADIR "${COMPIZ_INSTALL_GCONF_SCHEMA_DIR}")
	endif (NOT COMPIZ_INSTALL_GCONF_SCHEMA_DIR)
    else ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "compiz" OR
	  "$ENV{BUILD_GLOBAL}" STREQUAL "true")

	if (NOT COMPIZ_INSTALL_GCONF_SCHEMA_DIR)
            set (PLUGIN_SCHEMADIR "$ENV{HOME}/.gconf/schemas")
        else (NOT COMPIZ_INSTALL_GCONF_SCHEMA_DIR)
	    set (PLUGIN_SCHEMADIR "${COMPIZ_INSTALL_GCONF_SCHEMA_DIR}")
	endif (NOT COMPIZ_INSTALL_GCONF_SCHEMA_DIR)

    endif ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "package")
endmacro (compiz_gconf_prepare_install_dirs)

function (compiz_install_gconf_schema _src _dst)
    find_program (GCONFTOOL_EXECUTABLE gconftool-2)
    mark_as_advanced (FORCE GCONFTOOL_EXECUTABLE)

    if (GCONFTOOL_EXECUTABLE AND NOT COMPIZ_DISABLE_SCHEMAS_INSTALL)
	install (CODE "
		if (\"\$ENV{USER}\" STREQUAL \"root\")
		    exec_program (${GCONFTOOL_EXECUTABLE}
			ARGS \"--get-default-source\"
			OUTPUT_VARIABLE ENV{GCONF_CONFIG_SOURCE})
		    exec_program (${GCONFTOOL_EXECUTABLE}
			ARGS \"--makefile-install-rule ${_src} > /dev/null\")
		else (\"\$ENV{USER}\" STREQUAL \"root\")
		    exec_program (${GCONFTOOL_EXECUTABLE}
			ARGS \"--install-schema-file=${_src} > /dev/null\")
		endif (\"\$ENV{USER}\" STREQUAL \"root\")
		")
    endif ()
    install (
	FILES "${_src}"
	DESTINATION "${_dst}"
    )
endfunction ()

# generate gconf schema
find_program (XSLTPROC_EXECUTABLE xsltproc)
mark_as_advanced (FORCE XSLTPROC_EXECUTABLE)

if (XSLTPROC_EXECUTABLE)
    compiz_gconf_prepare_install_dirs ()
    add_custom_command (
	OUTPUT "${CMAKE_BINARY_DIR}/generated/compiz-${COMPIZ_CURRENT_PLUGIN}.schemas"
	COMMAND ${XSLTPROC_EXECUTABLE}
		-o "${CMAKE_BINARY_DIR}/generated/compiz-${COMPIZ_CURRENT_PLUGIN}.schemas"
		${COMPIZ_GCONF_SCHEMAS_XSLT}
		${COMPIZ_CURRENT_XML_FILE}
	DEPENDS ${COMPIZ_CURRENT_XML_FILE}
    )

    compiz_install_gconf_schema ("${CMAKE_BINARY_DIR}/generated/compiz-${COMPIZ_CURRENT_PLUGIN}.schemas" ${PLUGIN_SCHEMADIR})
    list (APPEND COMPIZ_CURRENT_SOURCES_ADDS ${CMAKE_BINARY_DIR}/generated/compiz-${COMPIZ_CURRENT_PLUGIN}.schemas)
endif ()
