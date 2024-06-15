dnl
dnl Copyright (c) 2009-2010
dnl         Daniel Morales <daniel@daniel.com.uy>
dnl

dnl -------------------------------------------------------
dnl Version
dnl -------------------------------------------------------
XDT_VERSION_INIT([0.3.2], [git])

dnl -------------------------------------------------------
dnl Init autoconf
dnl -------------------------------------------------------
AC_COPYRIGHT([Copyright (c) 2009-2010 Daniel Morales <daniel@daniel.com.uy>.])
AC_INIT([thunar-shares-plugin], [xdt_version], [https://gitlab.xfce.org/thunar-plugins/thunar-shares-plugin])
AC_REVISION([xdt_version_build])
AC_CONFIG_SRCDIR([thunar-plugin/thunar-shares-plugin.c])
AC_PREREQ(2.69)
AC_CANONICAL_TARGET()
AC_CONFIG_MACRO_DIRS([m4])

dnl -------------------------------------------------------
dnl Init automake
dnl -------------------------------------------------------
AM_INIT_AUTOMAKE([1.9 dist-bzip2 tar-ustar no-dist-gzip foreign])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl -------------------------------------------------------
dnl Programs
dnl -------------------------------------------------------
AC_PROG_CC
AC_PROG_INSTALL

dnl -------------------------------------------------------
dnl Initialize libtool
dnl-------------------------------------------------------
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl -------------------------------------------------------
dnl Check headers
dnl -------------------------------------------------------
AC_CHECK_INCLUDES_DEFAULT
AC_CHECK_HEADERS([string.h sys/types.h sys/wait.h time.h sys/stat.h unistd.h]) 

dnl -------------------------------------------------------
dnl Packages checks
dnl -------------------------------------------------------
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.22.0])
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.26.0])
XDT_CHECK_PACKAGE([THUNARX], [thunarx-3], [1.0.1])

dnl -------------------------------------------------------
dnl Languages
dnl -------------------------------------------------------
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])
AC_SEARCH_LIBS([bind_textdomain_codeset], [intl],
  [AC_DEFINE([HAVE_BIND_TEXTDOMAIN_CODESET], [1], [Define to 1 if you have the 'bind_textdomain_codeset' function.])],
  [])

dnl -------------------------------------------------------
dnl Debugging support
dnl -------------------------------------------------------
XDT_FEATURE_DEBUG([xdt_debug_default])

dnl -------------------------------------------------------
dnl Check for linker optimizations
dnl -------------------------------------------------------
XDT_FEATURE_LINKER_OPTS()

AC_CONFIG_FILES([
	Makefile
	po/Makefile.in
	libshares/Makefile
	thunar-plugin/Makefile
])
AC_OUTPUT()

echo
echo "Prefix:        $prefix"
echo "Debug Support: $enable_debug"
echo
