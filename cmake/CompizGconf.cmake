option (
    COMPIZ_DISABLE_SCHEMAS_INSTALL
    "Disables gconf schema installation with gconftool"
    OFF
)

set (
    COMPIZ_INSTALL_GCONF_SCHEMA_DIR ${COMPIZ_INSTALL_GCONF_SCHEMA_DIR} CACHE PATH
    "Installation path of the gconf schema file"
)

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
function (compiz_gconf_schema _src _dst _inst)
    find_program (XSLTPROC_EXECUTABLE xsltproc)
    mark_as_advanced (FORCE XSLTPROC_EXECUTABLE)

    if (XSLTPROC_EXECUTABLE)
	add_custom_command (
	    OUTPUT ${_dst}
	    COMMAND ${XSLTPROC_EXECUTABLE}
		    -o ${_dst}
		    ${COMPIZ_GCONF_SCHEMAS_XSLT}
		    ${_src}
	    DEPENDS ${_src}
	)
	compiz_install_gconf_schema (${_dst} ${_inst})
    endif ()
endfunction ()
