project('tmxlite', 'cpp', 'c',
  version: '1.4.4',
  meson_version: '>=0.49.2',
  default_options: [
	'buildtype=release',
	'cpp_std=c++14',
	'warning_level=3',
	'b_ndebug=if-release'
  ]
)

cpp = meson.get_compiler('cpp')
if get_option('use_rtti')
  if cpp.get_argument_syntax() == 'gcc'
	if get_option('project_static_runtime')
	  add_project_arguments('-static', language: ['cpp', 'c'])
	endif
  endif
else
  if cpp.get_argument_syntax() == 'gcc'
	if get_option('project_static_runtime')
	  add_project_arguments('-fno-rtti', '-static', language: ['cpp', 'c'])
	else
	  add_project_arguments('-fno-rtti', language: ['cpp', 'c'])
	endif
  endif
endif

binary_postfix = ''
if get_option('default_library') == 'static'
  if get_option('debug')
	add_project_arguments('-D_DEBUG_', '-DTMXLITE_STATIC', language: ['cpp', 'c'])
	binary_postfix = '-s-d'
  else
	add_project_arguments('-O3', '-DTMXLITE_STATIC', language: ['cpp', 'c'])
	binary_postfix = '-s'
  endif
elif get_option('default_library') == 'shared'
  if get_option('debug')
	add_project_arguments('-D_DEBUG_', language: ['cpp', 'c'])
	binary_postfix = '-d'
  else
	add_project_arguments('-O3', language: ['cpp', 'c'])
  endif
else
  error('Unsupported option \'' + get_option('default_library') + '\' for project ' + meson.project_name())
endif

if cpp.get_argument_syntax() == 'msvc'
  add_project_arguments('-D_CRT_SECURE_NO_WARNINGS', language: ['cpp', 'c'])
endif

if get_option('use_extlibs')
  add_project_arguments('-DUSE_EXTLIBS', language: ['cpp', 'c'])
  zdep = dependency('zlib', version : '>=1.2.8', required: true)
  pugidep = dependency('pugixml', required: true)
  zstddep = dependency('zstd', required: true)
else
  if get_option('use_zstd')
    add_project_arguments('-DUSE_ZSTD', language: ['cpp', 'c'])
    zstddep = dependency('zstd', required: true)
  endif
endif

subdir('tmxlite')
if get_option('build_examples')
  subdir('OpenGLExample')
  subdir('SFMLExample')
endif
if get_option('build_tests')
  subdir('ParseTest')
endif
