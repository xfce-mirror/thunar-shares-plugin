tsp_sources = [
  'thunar-shares-plugin.c',
  'tsp-page.c',
  'tsp-page.h',
  'tsp-provider.c',
  'tsp-provider.h',
]

shared_module(
  'thunar-shares-plugin',
  tsp_sources,
  gnu_symbol_visibility: 'hidden',
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    glib,
    gtk,
    thunarx,
    xfconf,
  ],
  link_with: [
    libshares,
  ],
  name_prefix: '',
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / 'thunarx-3',
)
