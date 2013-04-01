# - Try to find OpenGLES
# Once done this will define
#  
#  OPENGLES2_FOUND        - system has OpenGLES
#  OPENGLES2_INCLUDE_DIR  - the GLES include directory
#  OPENGLES2_LIBRARY	  - the GLES library
#  OPENGLES2_LIBRARIES    - Link this to use OpenGLES
#   

FIND_PATH(OPENGLES2_INCLUDE_DIR GLES2/gl2.h
  /usr/openwin/share/include
  /opt/graphics/OpenGL/include /usr/X11R6/include
  /usr/include
)

FIND_LIBRARY(OPENGLES2_LIBRARY
  NAMES GLESv2
  PATHS /opt/graphics/OpenGL/lib
        /usr/openwin/lib
        /usr/shlib /usr/X11R6/lib
        /usr/lib
)

FIND_LIBRARY(OPENGLES2_EGL_LIBRARY
    NAMES EGL
    PATHS /usr/shlib /usr/X11R6/lib
          /usr/lib
)

# On Unix OpenGL most certainly always requires X11.
# Feel free to tighten up these conditions if you don't 
# think this is always true.
# It's not true on OSX.

IF (OPENGLES2_LIBRARY)
  IF(NOT X11_FOUND)
    INCLUDE(FindX11)
  ENDIF(NOT X11_FOUND)
  IF (X11_FOUND)
    IF (NOT APPLE)
      SET (OPENGLES2_LIBRARIES ${X11_LIBRARIES})
    ENDIF (NOT APPLE)
  ENDIF (X11_FOUND)
ENDIF(OPENGLES2_LIBRARY)

SET( OPENGLES2_FOUND "NO" )
IF(OPENGLES2_LIBRARY AND OPENGLES2_EGL_LIBRARY)
    SET( OPENGLES2_LIBRARIES  ${OPENGLES2_LIBRARY} ${OPENGLES2_EGL_LIBRARY} ${OPENGLES2_LIBRARIES})
    SET( OPENGLES2_FOUND "YES" )
ENDIF(OPENGLES2_LIBRARY AND OPENGLES2_EGL_LIBRARY)

