pvt_incdir = include_directories('src')

conf = configuration_data()
conf.set('ASSETS_PATH', meson.current_source_dir() / '')
if get_option('pause_test')
  conf.set('PAUSE_AT_END', true)
endif

project_config_file = configure_file(
	input: 'src/config.h.in',
	output: 'config.h',
	configuration: conf,
	format: 'cmake@'
)

exec_target = executable('tmxlite_parse_test',
  'src/main.cpp',
  project_config_file,
  install: false,
  include_directories: pvt_incdir,
  dependencies: [
	tmxlite_dep
  ],
)

test(meson.project_name() + ' unit test', exec_target)
