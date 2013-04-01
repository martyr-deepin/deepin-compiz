
# modify pkg-config libs for opengl based on if we found GLES or not
if (${COMPIZ_CURRENT_PLUGIN} STREQUAL "opengl")
    if (USE_GLES)
	set (PKGCONFIG_LIBS "-lGLESv2 -lEGL")
    else (USE_GLES)
	set (PKGCONFIG_LIBS "-lGL")
    endif (USE_GLES)
endif (${COMPIZ_CURRENT_PLUGIN} STREQUAL "opengl")

# if plugin is using opengl plugin check for GLES library and set correct define
if (NOT "${${_PLUGIN}_PLUGINDEPS}" STREQUAL "")
    string (REGEX MATCH "opengl" opengl_found ${${_PLUGIN}_PLUGINDEPS})

    if (opengl_found STREQUAL "opengl")
	if (USE_GLES)
	    set (${_PLUGIN}_CFLAGSADD ${${_PLUGIN}_CFLAGSADD} " -DUSE_GLES")
	    string (REPLACE ";" " " ${_PLUGIN}_CFLAGSADD ${${_PLUGIN}_CFLAGSADD})
	endif (USE_GLES)
    endif (opengl_found STREQUAL "opengl")
endif (NOT "${${_PLUGIN}_PLUGINDEPS}" STREQUAL "")

