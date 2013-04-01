find_package (Compiz REQUIRED)
include (CompizPlugin)
include (FindJPEG)

if (JPEG_FOUND)
    compiz_plugin (imgjpeg LIBRARIES ${JPEG_LIBRARIES} INCDIRS ${JPEG_INCLUDE_DIR})
endif (JPEG_FOUND)
