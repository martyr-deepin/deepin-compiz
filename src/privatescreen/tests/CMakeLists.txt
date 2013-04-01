include(CompizTesting)

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR} 

  ${compiz_BINARY_DIR}/generated
  ${compiz_SOURCE_DIR}/build/generated
  ${compiz_SOURCE_DIR}/include

  ${compiz_SOURCE_DIR}/src
  ${compiz_SOURCE_DIR}/src/timer/include
  ${compiz_SOURCE_DIR}/src/timer/src

  ${compiz_SOURCE_DIR}/src/rect/include
  ${compiz_SOURCE_DIR}/src/region/include
  ${compiz_SOURCE_DIR}/src/screen/geometry/include
  ${compiz_SOURCE_DIR}/src/window/geometry/include
  ${compiz_SOURCE_DIR}/src/window/extents/include
  ${compiz_SOURCE_DIR}/src/screen/extents/include
  ${compiz_SOURCE_DIR}/src/servergrab/include

  ${compiz_SOURCE_DIR}/src/pluginclasshandler/include

  ${COMPIZ_INCLUDE_DIRS}

  ${Boost_INCLUDE_DIRS}
)


add_definitions (
    -DHAVE_CONFIG_H
    -DPLUGINDIR=\\\"${compiz_plugindir}\\\"
    -DSHAREDIR=\\\"${compiz_sharedir}\\\"
    -DMETADATADIR=\\\"${compiz_metadatadir}\\\"
)


add_executable( 
  compiz_privatescreen_test
  
  ${CMAKE_CURRENT_SOURCE_DIR}/test-privatescreen.cpp
)

target_link_libraries( 
  compiz_privatescreen_test
  
  compiz_core
  compiz_window_constrainment
  compiz_logmessage
  
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
)

compiz_discover_tests (compiz_privatescreen_test COVERAGE compiz_core)
