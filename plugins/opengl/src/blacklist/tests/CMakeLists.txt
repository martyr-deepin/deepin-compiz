include_directories (${GTEST_INCLUDE_DIRS} ..)
set (exe "compiz_opengl_test_blacklist")
add_executable (${exe} test-blacklist.cpp)
target_link_libraries (${exe}
    compiz_opengl_blacklist
    ${GTEST_BOTH_LIBRARIES}
)
compiz_discover_tests(${exe} COVERAGE compiz_opengl_blacklist)

