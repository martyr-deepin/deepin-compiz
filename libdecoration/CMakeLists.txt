compiz_pkg_check_modules (HAVE_XRENDER_0_9_3 xrender>=0.9.3)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.libdecoration.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories (
    ${compiz_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBDECORATION_INCLUDE_DIRS}
)

add_definitions (
    -DHAVE_CONFIG_H
)

link_directories (
    ${LIBDECORATION_LINK_DIRS}
)

add_library (decoration SHARED
    decoration.c
)

set_target_properties (decoration PROPERTIES
    VERSION 0.0.0
    SOVERSION 0
)

target_link_libraries (
    decoration ${LIBDECORATION_LIBRARIES} m
)

install (
    TARGETS decoration
    DESTINATION ${libdir}
)

set (DECORATION_LIBS "")
foreach (_val ${LIBDECORATION_LDFLAGS})
    set (DECORATION_LIBS "${DECORATION_LIBS}${_val} ")
endforeach (_val ${LIBDECORATION_LDFLAGS})

set (DECORATION_CFLAGS "")
foreach (_val ${LIBDECORATION_CFLAGS})
    set (DECORATION_CFLAGS "${DECORATION_CFLAGS}${_val} ")
endforeach (_val ${LIBDECORATION_CFLAGS})

compiz_configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/libdecoration.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libdecoration.pc
)

install (
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libdecoration.pc
    DESTINATION ${libdir}/pkgconfig
)
