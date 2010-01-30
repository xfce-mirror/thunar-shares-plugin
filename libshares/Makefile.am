
INCLUDES =						\
	-I$(top_builddir)				\
	-I$(top_srcdir)					\
	-DG_LOG_DOMAIN=\"thunar-shares-plugin-lib\"	\
	-DLOCALEDIR=\"$(localedir)\"			\
	-DDATADIR=\"$(datadir)\"			\
	$(PLATFORM_CPPFLAGS)

noinst_LTLIBRARIES = libshares.la

libshares_la_SOURCES =					\
	shares.c		shares.h		\
	libshares-util.c	libshares-util.h

libshares_la_CFLAGS =					\
	$(PLATFORM_CFLAGS)				\
	$(GTK_CFLAGS)					\
	$(GLIB_CFLAGS)					\
	$(THUNARX_CFLAGS)

libshares_la_LDFLAGS =					\
	$(GTK_LIBS)					\
	$(GLIB_LIBS)					\
	$(THUNARX_LIBS)					\
	$(PLATFORM_LDFLAGS)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
