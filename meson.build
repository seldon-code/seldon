project('seldon', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

incdir = include_directories('include')

sources_seldon = [
  'src/simulation.cpp',
  'src/network.cpp',
  'src/model.cpp',
  'src/models/DeGroot.cpp',
  'src/util/tomlplusplus.cpp'
]

exe = executable('seldon', sources_seldon + 'src/main.cpp',
  install : true,
  dependencies : [dependency('fmt'), dependency('tomlplusplus')],
  include_directories : incdir
  )


tests = [
  ['Test A', 'test/test_a.cpp'],
  ['Test B', 'test/test_b.cpp']
]

Catch2 = dependency('Catch2', method : 'cmake', modules : ['Catch2::Catch2WithMain', 'Catch2::Catch2'])

foreach t : tests
  exe = executable(t.get(0), sources_seldon + t.get(1),
    dependencies : [dependency('fmt'), dependency('tomlplusplus'), Catch2],
    include_directories : incdir
  )
  test(t.get(0), exe)
endforeach