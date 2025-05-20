sfml_graphics_dep = dependency('sfml-graphics')
sfml_window_dep = dependency('sfml-window')
sfml_system_dep = dependency('sfml-system')

pvt_incdir = include_directories('src')

executable('SFML_example',
  'src/main.cpp',
  install: false,
  include_directories: pvt_incdir,
  dependencies: [
	tmxlite_dep,
	sfml_graphics_dep,
	sfml_window_dep,
	sfml_system_dep,
  ],
)
