find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (session PKGDEPS libxml-2.0)
