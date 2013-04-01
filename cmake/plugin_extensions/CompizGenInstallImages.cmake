# determinate installation directories
macro (compiz_images_prepare_dirs)
    if ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "package")
	set (PLUGIN_IMAGEDIR   ${datadir}/compiz/${COMPIZ_CURRENT_PLUGIN})

    elseif ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "compiz" OR
	    "$ENV{BUILD_GLOBAL}" STREQUAL "true")
	set (PLUGIN_IMAGEDIR   ${COMPIZ_PREFIX}/share/compiz/${COMPIZ_CURRENT_PLUGIN})

    else ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "compiz" OR
	  "$ENV{BUILD_GLOBAL}" STREQUAL "true")
	set (PLUGIN_IMAGEDIR   $ENV{HOME}/.compiz-1/${COMPIZ_CURRENT_PLUGIN})

    endif ("${COMPIZ_PLUGIN_INSTALL_TYPE}" STREQUAL "package")
endmacro (compiz_images_prepare_dirs)

# install plugin data files
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/images)
    compiz_images_prepare_dirs ()
    install (
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/images
	DESTINATION ${PLUGIN_IMAGEDIR}
    )
    list (APPEND COMPIZ_DEFINITIONS_ADD "-DIMAGEDIR='\"${PLUGIN_IMAGEDIR}\"'")
endif ()
