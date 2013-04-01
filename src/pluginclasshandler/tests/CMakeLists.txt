include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_library( compiz_pch_test STATIC test-pluginclasshandler.cpp )

add_executable( 
  compiz_pch_construct
  
  ${CMAKE_CURRENT_SOURCE_DIR}/construct/src/test-pch-construct.cpp
)

add_executable( 
  compiz_pch_get

  ${CMAKE_CURRENT_SOURCE_DIR}/get/src/test-pch-get.cpp
)

#add_executable( 
#  compiz_pch_indexes
#  
#  ${CMAKE_CURRENT_SOURCE_DIR}/indexes/src/test-pch-indexes.cpp
#)

add_executable( 
  compiz_pch_typenames
  
  ${CMAKE_CURRENT_SOURCE_DIR}/typenames/src/test-pch-typenames.cpp
)

target_link_libraries( 
  compiz_pch_construct 
  compiz_pch_test
  
  compiz_logmessage
  compiz_pluginclasshandler 
  compiz_string 
  
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
)

target_link_libraries( 
  compiz_pch_get 
  compiz_pch_test
  
  compiz_logmessage
  compiz_pluginclasshandler 
  compiz_string 
  
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
)

target_link_libraries( 
  compiz_pch_typenames
  compiz_pch_test
  
  compiz_logmessage
  compiz_pluginclasshandler 
  compiz_string 
  
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
)

# Not compilable
#target_link_libraries( 
#  compiz_pch_indexes
#  compiz_pch_test
#  
#  compiz_pluginclasshandler 
#  compiz_string 
#  
#  gtest
#  gtest_main
#)

compiz_discover_tests (compiz_pch_construct COVERAGE compiz_pluginclasshandler)
compiz_discover_tests (compiz_pch_get COVERAGE compiz_pluginclasshandler)
#add_test( compiz_pch_indexes compiz_pch_indexes )
compiz_discover_tests (compiz_pch_typenames COVERAGE compiz_pluginclasshandler)

