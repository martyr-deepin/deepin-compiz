set (_files
    LibCompizConfigCommon.cmake
)

install (
    FILES ${_files}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/compiz/cmake
)
