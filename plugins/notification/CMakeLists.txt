find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (notification PKGDEPS libnotify gtk+-2.0)
compiz_pkg_check_modules (HAVE_LIBNOTIFY_0_6_1 libnotify>=0.6.1)
if (HAVE_LIBNOTIFY_0_6_1_FOUND)
	add_definitions (-DHAVE_LIBNOTIFY_0_6_1)
endif (HAVE_LIBNOTIFY_0_6_1_FOUND)
