include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
)

add_executable( 
  compiz_wrapsystem_test
  
  ${CMAKE_CURRENT_SOURCE_DIR}/test-wrapsystem.cpp
)

target_link_libraries( 
  compiz_wrapsystem_test
  
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
)

compiz_discover_tests (compiz_wrapsystem_test COVERAGE compiz_core)