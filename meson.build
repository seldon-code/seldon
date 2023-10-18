project('seldon', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

incdir = include_directories('include')

sources_seldon = [
  'src/simulation.cpp',
  'src/network.cpp',
  'src/network_generation.cpp',
  'src/model.cpp',
  'src/models/DeGroot.cpp',
  'src/models/ActivityDrivenModel.cpp',
  'src/util/tomlplusplus.cpp'
]

exe = executable('seldon', sources_seldon + 'src/main.cpp',
  install : true,
  dependencies : [dependency('fmt'), dependency('tomlplusplus')],
  include_directories : incdir
  )


tests = [
  ['Test Tarjan', 'test/test_tarjan.cpp'],
  ['Test DeGroot', 'test/test_deGroot.cpp'],
  ['Test Network', 'test/test_network.cpp'],
  ['Test Sampling', 'test/test_sampling.cpp'],
]

Catch2 = dependency('Catch2', method : 'cmake', modules : ['Catch2::Catch2WithMain', 'Catch2::Catch2'])

foreach t : tests
  exe = executable(t.get(0), sources_seldon + t.get(1),
    dependencies : [dependency('fmt'), dependency('tomlplusplus'), Catch2],
    include_directories : incdir
  )
  test(t.get(0), exe)
endforeach