project('seldon', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20', 'optimization=3'])

add_global_arguments(['-Wno-unused-local-typedefs', '-Wno-array-bounds'], language : 'cpp')

incdir = include_directories('include')

sources_seldon = [
  'src/config_parser.cpp',
  'src/models/DeGroot.cpp',
  'src/models/ActivityDrivenModel.cpp',
  'src/util/tomlplusplus.cpp',
]

exe = executable('seldon', sources_seldon + 'src/main.cpp',
  install : true,
  dependencies : [dependency('fmt'), dependency('tomlplusplus')],
  include_directories : incdir
  )


tests = [
  ['Test_Tarjan', 'test/test_tarjan.cpp'],
  ['Test_DeGroot', 'test/test_deGroot.cpp'],
  ['Test_Activity_Driven', 'test/test_activity.cpp'],
  ['Test_Network', 'test/test_network.cpp'],
  ['Test_Network_Generation', 'test/test_network_generation.cpp'],
  ['Test_Sampling', 'test/test_sampling.cpp'],
  ['Test_IO', 'test/test_io.cpp'],
]

Catch2 = dependency('Catch2', method : 'cmake', modules : ['Catch2::Catch2WithMain', 'Catch2::Catch2'])

foreach t : tests
  exe = executable(t.get(0), sources_seldon + t.get(1),
    dependencies : [dependency('fmt'), dependency('tomlplusplus'), Catch2],
    include_directories : incdir
  )
  test(t.get(0), exe, workdir : meson.project_source_root())
endforeach

benchmarks = [
  ['Benchmark_ActivityDriven_Model','benchmark/benchmark_activity.cpp'],
  ['Benchmark_deGroot_Model','benchmark/benchmark_deGroot.cpp'],
]

foreach b : benchmarks
  exe = executable(b.get(0), sources_seldon + b.get(1),
    dependencies : [dependency('fmt'), dependency('tomlplusplus'),Catch2,dependency('benchmark')],
    include_directories : incdir
  )
  benchmark(b.get(0), exe, workdir : meson.project_source_root())
endforeach