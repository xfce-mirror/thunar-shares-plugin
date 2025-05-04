libshares_sources = [
  'libshares-util.c',
  'libshares-util.h',
  'shares.c',
  'shares.h',
]

libshares = static_library(
  'libshares',
  libshares_sources,
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    glib,
    gtk,
    thunarx,
  ],
  install: false,
)
