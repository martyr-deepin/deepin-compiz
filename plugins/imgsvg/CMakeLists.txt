find_package (Compiz REQUIRED)

include (CompizPlugin)

pkg_check_modules (RSVG librsvg-2.0>=2.36.2)

if (RSVG_FOUND)
    compiz_set (HAVE_RSVG_2_36_2 1)
endif (RSVG_FOUND)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/src/imgsvg-config.h.in
		${CMAKE_CURRENT_BINARY_DIR}/src/imgsvg-config.h)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/src)

compiz_plugin(imgsvg PLUGINDEPS composite opengl PKGDEPS cairo>=1.0 cairo-xlib librsvg-2.0>=2.14.0 LIBRARIES decoration)
