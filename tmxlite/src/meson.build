tmxlite_lib = library(meson.project_name() + binary_postfix,
  'detail/pugixml.cpp',
  'FreeFuncs.cpp',
  'ImageLayer.cpp',
  'Map.cpp',
  'miniz.c',
  'Object.cpp',
  'ObjectGroup.cpp',
  'Property.cpp',
  'TileLayer.cpp',
  'LayerGroup.cpp',
  'Tileset.cpp',
  install: true,
  include_directories: incdir,
)

tmxlite_dep = declare_dependency(
  link_with: tmxlite_lib,
  include_directories: incdir,
)
