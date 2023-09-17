project('seldon', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

incdir = include_directories('include')

exe = executable('seldon', 'src/main.cpp',
  install : true,
  dependencies : [dependency('fmt')],
  include_directories : incdir
  )

