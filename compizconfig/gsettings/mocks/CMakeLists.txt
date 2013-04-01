pkg_check_modules (GLIB glib-2.0)

if (GLIB_FOUND)

    include_directories (${CMAKE_CURRENT_SOURCE_DIR}
			 ${CMAKE_CURRENT_BINARY_DIR}
			 ${CMAKE_CURRENT_SOURCE_DIR}/../../libcompizconfig/include
			 ${CMAKE_CURRENT_SOURCE_DIR}/../gsettings_backend_shared
			 ${GLIB_INCLUDE_DIRS})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

    add_library (compizconfig_ccs_gsettings_wrapper_mock STATIC
	         ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_wrapper_mock.cpp
	         ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_wrapper_mock.h)

    add_library (compizconfig_ccs_gsettings_backend_mock STATIC
	         ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_backend_mock.cpp
	         ${CMAKE_CURRENT_SOURCE_DIR}/ccs_gsettings_backend_mock.h)

endif (GLIB_FOUND)
