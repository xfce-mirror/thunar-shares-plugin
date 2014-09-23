
INCLUDES =							\
	-I$(top_builddir)					\
	-I$(top_srcdir)						\
	-DG_LOG_DOMAIN=\"thunar-shares-plugin\"			\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"			\
	-DDATADIR=\"$(datadir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/thunarx-2
extensions_LTLIBRARIES = thunar-shares-plugin.la

thunar_shares_plugin_la_SOURCES =				\
	thunar-shares-plugin.c					\
	tsp-page.c		tsp-page.h			\
	tsp-provider.c		tsp-provider.h

thunar_shares_plugin_la_CFLAGS =				\
	$(PLATFORM_CFLAGS)					\
	$(GTK_CFLAGS)						\
	$(GLIB_CFLAGS)						\
	$(THUNARX_CFLAGS)

thunar_shares_plugin_la_LDFLAGS =				\
	-avoid-version						\
	-export-dynamic						\
	-module							\
	$(GTK_LIBS)						\
	$(GLIB_LIBS)						\
	$(THUNARX_LIBS)						\
	$(PLATFORM_LDFLAGS)

thunar_shares_plugin_la_LIBADD=					\
	$(top_builddir)/libshares/libshares.la

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
