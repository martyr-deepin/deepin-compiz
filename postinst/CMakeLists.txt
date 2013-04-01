# Allow setting the recompile rule again
set_property (GLOBAL
	      PROPERTY COMPIZ_INHIBIT_ADD_INSTALL_RECOMPILE_RULE
	      FALSE)
set_property (GLOBAL
	      PROPERTY GSETTINGS_LOCAL_COMPILE_INHIBIT_RULE
	      FALSE)

if (USE_GSETTINGS)

    compiz_add_install_recompile_gsettings_schemas (${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)
    add_all_gsettings_schemas_to_local_recompilation_rule ()

endif (USE_GSETTINGS)
