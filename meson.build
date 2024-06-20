project('seldon', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20', 'optimization=3'])

cppc = meson.get_compiler('cpp')

# Add C++ compiler options
_incdir = []  # Include directories
_deps = []  # Dependencies
_args = []  # Extra arguments


_incdir += include_directories('include')
_deps += [dependency('fmt'), dependency('tomlplusplus')]
_args +=  cppc.get_supported_arguments(['-Wno-unused-local-typedefs', '-Wno-array-bounds'])

sources_seldon = [
  'src/config_parser.cpp',
  'src/models/DeGroot.cpp',
  'src/models/ActivityDrivenModel.cpp',
  'src/models/DeffuantModel.cpp',
  'src/models/DeffuantModelVector.cpp',
  'src/models/InertialModel.cpp',
  'src/util/tomlplusplus.cpp',
]

# both static and shared library for bindings
# -----------------------------------

symbol_visibility = 'default'
if get_option('default_library') == 'static'
  # from https://github.com/ERGO-Code/HiGHS/pull/1737/files
  symbol_visibility = 'inlineshidden'
endif

seldon_lib = both_libraries('seldon',
  sources_seldon,
  install:true,
  dependencies: _deps,
  include_directories:_incdir,
  gnu_symbol_visibility: symbol_visibility,
  pic: true,
  cpp_args : _args,
)

seldon_static_dep = declare_dependency(include_directories:_incdir,
  link_with : seldon_lib.get_static_lib(), dependencies: _deps)
seldon_shared_dep = declare_dependency(include_directories : _incdir,
  link_with : seldon_lib.get_shared_lib(), dependencies: _deps)

# ------------------------------------

if get_option('build_exe')
  exe = executable('seldon', sources_seldon + 'src/main.cpp',
    install : true,
    dependencies : _deps,
    include_directories : _incdir,
    cpp_args : _args
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
  _deps+= Catch2

  foreach t : tests
    exe = executable(t.get(0), sources_seldon + t.get(1),
      dependencies : _deps,
      include_directories : _incdir,
      cpp_args : _args
    )
    test(t.get(0), exe, workdir : meson.project_source_root())
  endforeach
endif