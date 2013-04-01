find_package (Compiz REQUIRED)

compiz_plugin (ccp LIBRARIES compizconfig CFLAGSADD -I${CMAKE_SOURCE_DIR}/compizconfig/libcompizconfig/include)

if (NOT COMPIZ_DISABLE_PLUGIN_CCP)
    if (COMPIZ_BUILD_WITH_RPATH)
	set_target_properties (
	    ccp PROPERTIES
	    INSTALL_RPATH "${COMPIZCONFIG_LIBDIR}"
	)
    endif (COMPIZ_BUILD_WITH_RPATH)
endif (NOT COMPIZ_DISABLE_PLUGIN_CCP)
