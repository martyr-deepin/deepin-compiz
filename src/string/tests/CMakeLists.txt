include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR} 
)

add_executable( 
  compiz_string_test
  
  ${CMAKE_CURRENT_SOURCE_DIR}/printf/src/test-string-printf.cpp
)

target_link_libraries( 
  compiz_string_test 
  
  compiz_string
  
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
)

compiz_discover_tests (compiz_string_test COVERAGE compiz_string)
