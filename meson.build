project('seldon', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

incdir = include_directories('include')

sources = [
  'src/state.cpp',
  'src/main.cpp',
  'src/network.cpp',
  'src/model.cpp',
  'src/models/DeGroot.cpp'
]

exe = executable('seldon', sources,
  install : true,
  dependencies : [dependency('fmt')],
  include_directories : incdir
  )