sdl2_dep = dependency('sdl2')
sdl2image_dep = dependency('SDL2_image')
opengl_dep = dependency('opengl')
glm_dep = dependency('glm')
glew_dep = dependency('glew')

pvt_incdir = include_directories('src')

executable('OpenGL_example',
  'src/Game.cpp',
  'src/main.cpp',
  'src/MapLayer.cpp',
  'src/OpenGL.cpp',
  install: false,
  include_directories: pvt_incdir,
  dependencies: [
	tmxlite_dep,
	sdl2_dep,
	sdl2image_dep,
	opengl_dep,
	glew_dep,
  ],
)
