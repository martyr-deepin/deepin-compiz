set (COMPIZ_LD_LIBRARY_PATH ${CMAKE_BINARY_DIR}/src CACHE STRING "" FORCE)
set (COMPIZ_BINARY ${CMAKE_BINARY_DIR}/src/compiz CACHE STRING "" FORCE)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/compiz-xorg-gtest-config.h.in
		${CMAKE_CURRENT_BINARY_DIR}/compiz-xorg-gtest-config.h
		@ONLY)

add_definitions (${_xorg_gtest_cflags})

include_directories (${COMPIZ_XORG_SYSTEM_TEST_INCLUDE_DIR}
		     ${XORG_SERVER_INCLUDE_DIRS}
		     ${GTEST_INCLUDE_DIRS}
		     ${XORG_SERVER_INCLUDE_XORG_GTEST}
		     ${XORG_SERVER_GTEST_SRC}
		     ${CMAKE_CURRENT_BINARY_DIR})

link_directories (${XORG_SERVER_LIBRARY_DIRS})

# This actually includes xorg-gtest-all and the defines
set (_xorg_gtest_all_srcs
     ${XORG_SERVER_GTEST_SRC}/src/xorg-gtest-all.cpp)

set (_xorg_gtest_main_srcs
     ${XORG_SERVER_GTEST_SRC}/src/xorg-gtest_main.cpp)

add_library (xorg_gtest_all STATIC
	     ${_xorg_gtest_all_srcs})

add_library (xorg_gtest_main STATIC
	     ${_xorg_gtest_main_srcs})

add_library (compiz_xorg_gtest_system_test STATIC
	     ${CMAKE_CURRENT_SOURCE_DIR}/compiz-xorg-gtest.cpp)

target_link_libraries (xorg_gtest_all
		       ${GTEST_BOTH_LIBRARIES}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${XORG_SERVER_LIBRARIES})

target_link_libraries (xorg_gtest_main
		       ${GTEST_BOTH_LIBRARIES}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${XORG_SERVER_LIBRARIES})

target_link_libraries (compiz_xorg_gtest_system_test
		       xorg_gtest_all
		       xorg_gtest_main
		       ${GTEST_BOTH_LIBRARIES}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${XORG_SERVER_LIBRARIES}
		       ${X11_XI_LIBRARIES})
