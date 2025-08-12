[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://gitlab.xfce.org/thunar-plugins/thunar-shares-plugin/-/blob/master/COPYING)

# thunar-shares-plugin

The Thunar Shares Plugin (thunar-shares-plugin) allows you to quickly share a folder using Samba from Thunar (the Xfce file manager) without requiring root access.

----

### Homepage

[Thunar-shares-plugin documentation](https://docs.xfce.org/xfce/thunar/thunar-shares-plugin)

### Changelog

See [NEWS](https://gitlab.xfce.org/thunar-plugins/thunar-shares-plugin/-/blob/master/NEWS) for details on changes and fixes made in the current release.

### Source Code Repository

[Thunar-shares-plugin source code](https://gitlab.xfce.org/thunar-plugins/thunar-shares-plugin)

### Download a Release Tarball

[Thunar-shares-plugin shares](https://archive.xfce.org/src/thunar-plugins/thunar-shares-plugin)
    or
[Thunar-shares-plugin tags](https://gitlab.xfce.org/thunar-plugins/thunar-shares-plugin/-/tags)

### Installation

From source code repository: 

    % cd thunar-shares-plugin
    % meson setup build
    % meson compile -C build
    % meson install -C build

From release tarball:

    % tar xf thunar-shares-plugin-<version>.tar.xz
    % cd thunar-shares-plugin-<version>
    % meson setup build
    % meson compile -C build
    % meson install -C build

For information about how to setup Samba correctly, see [Samba Setup](https://docs.xfce.org/xfce/thunar/thunar-shares-plugin#samba_setup) in the Xfce documentation.

### Uninstallation

    % ninja uninstall -C build

### Reporting Bugs

Visit the [reporting bugs](https://docs.xfce.org/thunar-plugins/thunar-shares-plugin/bugs) page to view currently open bug reports and instructions on reporting new bugs or submitting bugfixes.

