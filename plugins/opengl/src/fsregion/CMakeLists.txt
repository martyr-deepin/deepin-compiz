if (COMPIZ_BUILD_TESTING)
add_subdirectory (tests)
endif ()

add_library (compiz_opengl_fsregion STATIC fsregion.cpp)
target_link_libraries (compiz_opengl_fsregion compiz_core)

