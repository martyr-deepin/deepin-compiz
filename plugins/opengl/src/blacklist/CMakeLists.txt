if (COMPIZ_BUILD_TESTING)
add_subdirectory (tests)
endif ()

add_library (compiz_opengl_blacklist STATIC blacklist.cpp)

