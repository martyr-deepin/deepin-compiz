find_package (Compiz REQUIRED)

include (CompizPlugin)

#find_package (OpenGL)

#if (OPENGL_GLU_FOUND)
#    compiz_plugin(blur PLUGINDEPS composite opengl LIBRARIES decoration ${OPENGL_glu_LIBRARY} INCDIRS ${OPENGL_INCLUDE_DIR})

#    if (COMPIZ_BUILD_WITH_RPATH AND NOT COMPIZ_DISABLE_PLUGIN_BLUR)
#	set_target_properties (
#	    blur PROPERTIES
#	    INSTALL_RPATH "${COMPIZ_LIBDIR}"
#	)
#    endif (COMPIZ_BUILD_WITH_RPATH AND NOT COMPIZ_DISABLE_PLUGIN_BLUR)
#endif ()
