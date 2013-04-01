link_directories (${COMPIZ_TEST_PLACE_LINK_DIRS})

include_directories (${COMPIZ_TEST_PLACE_INCLUDE_DIRS}
		     ${compiz_SOURCE_DIR}/include
		     ${compiz_BINARY_DIR}
		     ${compiz_BINARY_DIR}/generated
		     ${compiz_SOURCE_DIR}/src
		     ${compiz_SOURCE_DIR}/plugins/place/src
		     ${compiz_SOURCE_DIR}/src/window/geometry/include/
		     ${compiz_SOURCE_DIR}/src/window/geometry-saver/include/
		     ${compiz_SOURCE_DIR}/src/window/extents/include/
		     ${compiz_SOURCE_DIR}/src/point/include/
		     ${compiz_SOURCE_DIR}/src/rect/include/
		     ${COMPIZ_PLACE_TEST_BASE_DIR})

add_executable (test-place-clamp-to-workarea
		test-place-constrain-to-workarea.cpp
		../test-place.cpp 
		${compiz_SOURCE_DIR}/plugins/place/src/constrain-to-workarea.cpp
		${compiz_SOURCE_DIR}/src/size.cpp)

target_link_libraries (test-place-clamp-to-workarea
		       compiz_window_geometry
		       compiz_window_extents
		       compiz_point
		       compiz_rect
		       ${COMPIZ_TEST_PLACE_LIBRARIES})

add_test (test-place-clamp-to-workarea
	  ${CMAKE_CURRENT_BINARY_DIR}/test-place-clamp-to-workarea)
