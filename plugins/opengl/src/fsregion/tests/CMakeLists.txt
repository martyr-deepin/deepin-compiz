include_directories (${GTEST_INCLUDE_DIRS} ..)
set (exe "compiz_opengl_test_fsregion")
add_executable (${exe} test-fsregion.cpp)
target_link_libraries (${exe}
    compiz_opengl_fsregion
    compiz_core
    ${GTEST_BOTH_LIBRARIES}
)
compiz_discover_tests(${exe} COVERAGE compiz_opengl_fsregion)

