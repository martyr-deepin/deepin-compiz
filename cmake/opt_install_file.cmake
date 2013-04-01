set (src "" CACHE STRING "Source")
set (dst "" CACHE STRING "Destination")

message (\"-- Installing: ${src}\")
execute_process (
    COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${src}\" \"$ENV{DESTDIR}${dst}\"
    RESULT_VARIABLE _result
    OUTPUT_QUIET ERROR_QUIET
)
if (_result)
    message (\"-- Failed to install: ${dst}\")
endif ()
