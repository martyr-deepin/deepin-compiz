option (
    USE_GSETTINGS
    "Generate GSettings schemas"
    ON
)

option (
    COMPIZ_DISABLE_GS_SCHEMAS_INSTALL
    "Disables gsettings schema installation"
    OFF
)

set (
    COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR ${COMPIZ_INSTALL_GSETTINGS_SCHEMA_DIR} CACHE PATH
    "Installation path of the gsettings schema file"
)

# Detect global schemas install dir
find_program (PKG_CONFIG_TOOL pkg-config)

get_property (GSETTINGS_GLOBAL_INSTALL_DIR_SET
	      GLOBAL
	      PROPERTY GSETTINGS_GLOBAL_INSTALL_DIR
	      SET)

if (PKG_CONFIG_TOOL AND NOT GSETTINGS_GLOBAL_INSTALL_DIR_SET)

    mark_as_advanced (FORCE PKG_CONFIG_TOOL)

    # find out where schemas need to go if we are installing them systemwide
    execute_process (COMMAND ${PKG_CONFIG_TOOL} glib-2.0 --variable prefix  OUTPUT_VARIABLE GSETTINGS_GLIB_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    set (GSETTINGS_GLOBAL_INSTALL_DIR "${GSETTINGS_GLIB_PREFIX}/share/glib-2.0/schemas/")

    set_property (GLOBAL
		  PROPERTY GSETTINGS_GLOBAL_INSTALL_DIR
		  ${GSETTINGS_GLOBAL_INSTALL_DIR})

endif (PKG_CONFIG_TOOL AND NOT GSETTINGS_GLOBAL_INSTALL_DIR_SET)

function (compiz_add_install_recompile_gsettings_schemas _schemadir_user)

    get_property (GSETTINGS_GLOBAL_INSTALL_DIR_SET
		  GLOBAL
		  PROPERTY GSETTINGS_GLOBAL_INSTALL_DIR
		  SET)

    if (GSETTINGS_GLOBAL_INSTALL_DIR_SET)

	get_property (GSETTINGS_GLOBAL_INSTALL_DIR
		      GLOBAL
		      PROPERTY GSETTINGS_GLOBAL_INSTALL_DIR)

	# Recompile GSettings Schemas
	install (CODE "
		 execute_process (COMMAND cmake -DSCHEMADIR_USER=${_schemadir_user} -DSCHEMADIR_ROOT=${GSETTINGS_GLOBAL_INSTALL_DIR} -P ${COMPIZ_CMAKE_MODULE_PATH}/recompile_gsettings_schemas_in_dir_user_env.cmake)
		 ")

    endif (GSETTINGS_GLOBAL_INSTALL_DIR_SET)

endfunction (compiz_add_install_recompile_gsettings_schemas)

function (compiz_install_gsettings_schema _src _dst)
    find_program (PKG_CONFIG_TOOL pkg-config)
    find_program (GLIB_COMPILE_SCHEMAS glib-compile-schemas)
    mark_as_advanced (FORCE PKG_CONFIG_TOOL)
    mark_as_advanced (GLIB_COMPILE_SCHEMAS)

    # find out where schemas need to go if we are installing them systemwide
    execute_process (COMMAND ${PKG_CONFIG_TOOL} glib-2.0 --variable prefix  OUTPUT_VARIABLE GSETTINGS_GLIB_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    SET (GSETTINGS_GLOBAL_INSTALL_DIR "${GSETTINGS_GLIB_PREFIX}/share/glib-2.0/schemas/")

    if (PKG_CONFIG_TOOL AND
	GLIB_COMPILE_SCHEMAS AND NOT
	COMPIZ_DISABLE_SCHEMAS_INSTALL AND
	USE_GSETTINGS)

	# Install schema file
	install (CODE "
		 execute_process (COMMAND cmake -DFILE=${_src} -DINSTALLDIR_USER=${_dst} -DINSTALLDIR_ROOT=${GSETTINGS_GLOBAL_INSTALL_DIR} -P ${COMPIZ_CMAKE_MODULE_PATH}/copy_file_install_user_env.cmake)
		 ")

	get_property (COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE_SET
		      GLOBAL
		      PROPERTY COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE
		      SET)

	if (COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE_SET)

	    get_property (COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE
			  GLOBAL
			  PROPERTY COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE)

	else (COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE_SET)

	    set (COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE FALSE)

	endif (COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE_SET)

	if (NOT COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE)
	    compiz_add_install_recompile_gsettings_schemas (${_dst} ${GSETTINGS_GLOBAL_INSTALL_DIR})
	endif (NOT COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE)

    endif (PKG_CONFIG_TOOL AND
	   GLIB_COMPILE_SCHEMAS AND NOT
	   COMPIZ_DISABLE_SCHEMAS_INSTALL AND
	   USE_GSETTINGS)
endfunction ()

function (add_gsettings_local_recompilation_rule _schemas)

    get_property (GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE_SET
		  GLOBAL
		  PROPERTY GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE
		  SET)

    if (GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE_SET)
	get_property (GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE
		      GLOBAL
		      PROPERTY GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE)
    else (GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE_SET)
	set (GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE FALSE)
    endif (GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE_SET)

    get_property (GSETTINGS_LOCAL_COMPILE_TARGET_SET
		  GLOBAL
		  PROPERTY GSETTINGS_LOCAL_COMPILE_TARGET_SET
		  SET)

    if (NOT GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE AND
	NOT GSETTINGS_LOCAL_COMPILE_TARGET_SET)

	find_program (GLIB_COMPILE_SCHEMAS glib-compile-schemas)
	mark_as_advanced (GLIB_COMPILE_SCHEMAS)

	if (GLIB_COMPILE_SCHEMAS)

	    set (_compiled_gschemas ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/gschemas.compiled)

	    # Invalidate the rule
	    if (EXISTS ${_compiled_gschemas})
		execute_process (COMMAND rm ${_compiled_gschemas})
	    endif (EXISTS ${_compiled_gschemas})

	    add_custom_command (OUTPUT ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/gschemas.compiled
				COMMAND ${GLIB_COMPILE_SCHEMAS} --targetdir=${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/
				${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/
				COMMENT "Recompiling GSettings schemas locally"
				DEPENDS ${${_schemas}}
	    )

	    add_custom_target (compiz_gsettings_compile_local ALL
			       DEPENDS ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/gschemas.compiled)

	    set_property (GLOBAL
			  PROPERTY GSETTINGS_LOCAL_COMPILE_TARGET_SET
			  TRUE)

	endif (GLIB_COMPILE_SCHEMAS)

    endif (NOT GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE AND
	   NOT GSETTINGS_LOCAL_COMPILE_TARGET_SET)

endfunction ()

function (add_all_gsettings_schemas_to_local_recompilation_rule)

    get_property (GSETTINGS_LOCAL_COMPILE_SCHEMAS
		  GLOBAL
		  PROPERTY GSETTINGS_LOCAL_COMPILE_SCHEMAS)

    # Deferencing it appears to just give it the first schema, that's not what
    # we really want, so just pass the reference and double-dereference
    # internally
    add_gsettings_local_recompilation_rule (GSETTINGS_LOCAL_COMPILE_SCHEMAS)

endfunction ()

function (add_gsettings_schema_to_recompilation_list _schema_file_name)

    set_property (GLOBAL
		  APPEND
		  PROPERTY GSETTINGS_LOCAL_COMPILE_SCHEMAS
		  "${_schema_file_name}")

    add_all_gsettings_schemas_to_local_recompilation_rule ()

endfunction ()

# generate gconf schema
function (compiz_gsettings_schema _name _src _dst _inst)
    find_program (XSLTPROC_EXECUTABLE xsltproc)
    find_program (GLIB_COMPILE_SCHEMAS glib-compile-schemas)
    mark_as_advanced (FORCE XSLTPROC_EXECUTABLE)

    if (XSLTPROC_EXECUTABLE AND GLIB_COMPILE_SCHEMAS AND USE_GSETTINGS)
	add_custom_command (
	    OUTPUT ${_dst}
	    COMMAND ${XSLTPROC_EXECUTABLE}
		    -o ${_dst}
		    ${COMPIZ_GSETTINGS_SCHEMAS_XSLT}
		    ${_src}
	    DEPENDS ${_src}
	)

	add_custom_target (${_name}_gsettings_schema
			   DEPENDS ${_dst})

	compiz_install_gsettings_schema (${_dst} ${_inst})
	add_gsettings_schema_to_recompilation_list (${_name}_gsettings_schema)
    endif ()
endfunction ()
