include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${compiz_SOURCE_DIR}/include
  ${compiz_SOURCE_DIR}/src/pluginclasshandler/include
  ${compiz_SOURCE_DIR}/src/timer/include
  ${compiz_SOURCE_DIR}/src/logmessage/include
  ${compiz_SOURCE_DIR}/src/string/include
  ${compiz_SOURCE_DIR}/src/rect/include
  ${compiz_SOURCE_DIR}/src/region/include
  ${compiz_SOURCE_DIR}/src/point/include
  ${compiz_SOURCE_DIR}/src/window/geometry/include
  ${compiz_SOURCE_DIR}/src/window/extents/include
  ${COMPIZ_INCLUDE_DIRS}
)

add_executable( 
  compiz_option_test
  
  ${CMAKE_CURRENT_SOURCE_DIR}/option.cpp
)

target_link_libraries( 
  compiz_option_test 
  
  compiz_core
  
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
)

compiz_discover_tests ( compiz_option_test COVERAGE compiz_core )
