set (CMAKE_MODULE_PATH_ORIG ${CMAKE_MODULE_PATH})
set (LIBCOMPIZCONFIG_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LIBCOMPIZCONFIGZ_CMAKE_MODULE_PATH})

# Sorry for the kittens
include (${CMAKE_CURRENT_SOURCE_DIR}/../cmake/LibCompizConfigCommon.cmake)

compizconfig_backend (ini)
