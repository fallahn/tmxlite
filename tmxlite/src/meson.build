if get_option('use_extlibs')
    tmxlite_lib = library(meson.project_name() + binary_postfix,
      'FreeFuncs.cpp',
      'ImageLayer.cpp',
      'Map.cpp',
      'Object.cpp',
      'ObjectGroup.cpp',
      'Property.cpp',
      'TileLayer.cpp',
      'LayerGroup.cpp',
      'Tileset.cpp',
      install: true,
      include_directories: incdir,
      dependencies: [zdep, pugidep, zstddep]
    )
else

  if get_option('use_zstd')
  
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
      dependencies: zstddep
    )
  else

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
  endif
endif

tmxlite_dep = declare_dependency(
  link_with: tmxlite_lib,
  include_directories: incdir,
)
