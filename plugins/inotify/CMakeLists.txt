find_package (Compiz REQUIRED)

include (CompizPlugin)
include (CheckIncludeFile)

check_include_file ("sys/inotify.h" HAS_INOTIFY)

if (HAS_INOTIFY)
    compiz_plugin(inotify)
endif ()