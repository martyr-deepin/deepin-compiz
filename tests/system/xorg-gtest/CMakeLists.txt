include (FindPkgConfig)

pkg_check_modules (XORG_SERVER xorg-gtest xorg-server x11)

option (BUILD_XORG_GTEST "Build Xorg GTest integration tests" OFF)

if (XORG_SERVER_FOUND AND BUILD_XORG_GTEST)

    execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=prefix xorg-gtest OUTPUT_VARIABLE _xorg_gtest_prefix)
    execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=includedir xorg-gtest OUTPUT_VARIABLE _xorg_gtest_include_dir)
    execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=sourcedir xorg-gtest OUTPUT_VARIABLE _xorg_gtest_source_dir)
    execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=CPPflags xorg-gtest OUTPUT_VARIABLE _xorg_gtest_cflags)

    set (XORG_SERVER_INCLUDE_XORG_GTEST ${_xorg_gtest_include_dir} CACHE PATH "Path to Xorg GTest Headers")
    set (XORG_SERVER_GTEST_SRC ${_xorg_gtest_source_dir} CACHE PATH "Path to Xorg GTest Sources")
    set (COMPIZ_XORG_SYSTEM_TEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE PATH "Path to Compiz Xorg GTest Headers")
    
    message (STATUS "Found xorg-gtest sources at " ${XORG_SERVER_GTEST_SRC})

    add_subdirectory (src)
    add_subdirectory (tests)

else (XORG_SERVER_FOUND AND BUILD_XORG_GTEST)

    message (WARNING "Could not found xorg-gtest, not building system tests")
    set (BUILD_XORG_GTEST OFF)

endif (XORG_SERVER_FOUND AND BUILD_XORG_GTEST)
