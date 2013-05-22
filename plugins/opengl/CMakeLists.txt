find_package (Compiz REQUIRED)

include (CompizPlugin)

set (INTERNAL_LIBRARIES
    compiz_opengl_double_buffer
    compiz_opengl_fsregion
    compiz_opengl_blacklist
    compiz_opengl_glx_tfp_bind
)

add_subdirectory (src/doublebuffer)
add_subdirectory (src/fsregion)
add_subdirectory (src/blacklist)
add_subdirectory (src/glxtfpbind)

include_directories (src/glxtfpbind/include)

if (USE_GLES)
    compiz_plugin(opengl PLUGINDEPS composite CFLAGSADD "-DUSE_GLES" LIBRARIES ${OPENGLES2_LIBRARIES} ${INTERNAL_LIBRARIES} dl INCDIRS ${OPENGLES2_INCLUDE_DIR})
else (USE_GLES)
    find_package (OpenGL)
    if (OPENGL_FOUND)
	compiz_plugin(opengl PLUGINDEPS composite LIBRARIES ${OPENGL_gl_LIBRARY} ${INTERNAL_LIBRARIES} dl INCDIRS ${OPENGL_INCLUDE_DIR})
    endif (OPENGL_FOUND)
endif (USE_GLES)

