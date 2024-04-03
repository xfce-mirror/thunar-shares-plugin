#!/bin/sh
#-
# Copyright (c) 2009-2010 Jannis Pohlmann <jannis@xfce.org>
#

(type xdt-autogen) >/dev/null 2>&1 || {
  cat >&2 <<EOF
autogen.sh: You don't seem to have the Xfce development tools installed on
            your system, which are required to build this software.
            Please install the xfce4-dev-tools package first, it is available
            from http://www.xfce.org/.
EOF
  exit 1
}

XDT_AUTOGEN_REQUIRED_VERSION="4.17.0" exec xdt-autogen $@

# vi:set ts=2 sw=2 et ai:

