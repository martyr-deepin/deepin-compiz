set (_headers
    decoration.h
)

install (
    FILES ${_headers}
    DESTINATION ${includedir}/compiz
)

add_subdirectory (core)
