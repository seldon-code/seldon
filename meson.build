project('seldon', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20', 'optimization=3'])

deps = []
cpp_args = ['-Wno-unused-local-typedefs', '-Wno-array-bounds']
# add_global_arguments(['-Wno-unused-local-typedefs', '-Wno-array-bounds'], language : 'cpp')

incdir = include_directories('include')
deps += [dependency('fmt'), dependency('tomlplusplus')]

sources_seldon = [
  'src/config_parser.cpp',
  'src/models/DeGroot.cpp',
  'src/models/ActivityDrivenModel.cpp',
  'src/models/DeffuantModel.cpp',
  'src/models/DeffuantModelVector.cpp',
  'src/models/InertialModel.cpp',
  'src/util/tomlplusplus.cpp',
  'src/run_model.cpp'
]

# Static library for bindings
# -----------------------------------
symbol_visibility = 'default' 
# only generating static library
seldon_lib = static_library('seldon',
  sources_seldon,
  install:true,
  dependencies: deps,
  include_directories:incdir,
  gnu_symbol_visibility: symbol_visibility,
  pic: true,
  cpp_args : cpp_args,
)

seldon_static_dep = declare_dependency(include_directories:incdir,
  link_with : seldon_lib, dependencies: deps)
# ------------------------------------

if get_option('build_exe')
  exe = executable('seldon', sources_seldon + 'src/main.cpp',
    install : true,
    dependencies : deps,
    include_directories : incdir,
    cpp_args : cpp_args
    )
endif

if get_option('build_tests')
  tests = [
    ['Test_Tarjan', 'test/test_tarjan.cpp'],
    ['Test_DeGroot', 'test/test_deGroot.cpp'],
    ['Test_Activity_Driven', 'test/test_activity.cpp'],
    ['Test_Activity_Driven_Inertial', 'test/test_activity_inertial.cpp'],
    ['Test_Deffuant', 'test/test_deffuant.cpp'],
    ['Test_Network', 'test/test_network.cpp'],
    ['Test_Network_Generation', 'test/test_network_generation.cpp'],
    ['Test_Sampling', 'test/test_sampling.cpp'],
    ['Test_IO', 'test/test_io.cpp'],
    ['Test_Util', 'test/test_util.cpp'],
    ['Test_Prob', 'test/test_probability_distributions.cpp'],
  ]

  Catch2 = dependency('Catch2', method : 'cmake', modules : ['Catch2::Catch2WithMain', 'Catch2::Catch2'])
  deps+= Catch2

  foreach t : tests
    exe = executable(t.get(0), sources_seldon + t.get(1),
      dependencies : deps,
      include_directories : incdir,
      cpp_args : cpp_args
    )
    test(t.get(0), exe, workdir : meson.project_source_root())
  endforeach
endif