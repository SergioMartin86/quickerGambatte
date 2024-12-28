project('quickerMGBA','c','cpp',
  version: '1.0.0',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
  subproject_dir : 'extern'
)

# Loading dependencies
subdir('source')

# Do not build any targets if this is a subproject
if meson.is_subproject() == false

# Common application flags
commonCompileArgs = [ '-Wall', '-Wfatal-errors' ]

# Grabbing jaffarCommon dependency

jaffarCommonSubproject = subproject('jaffarCommon')
jaffarCommonDependency = jaffarCommonSubproject.get_variable('jaffarCommonDependency')

# Building playback tool

if get_option('buildPlayer') == true

  newMGBAPlayer = executable('newMGBAPlayer',
    'source/player.cpp',
    cpp_args            : [ commonCompileArgs, '-DNCURSES' ],
    dependencies        : [ newMGBADependency,
                            jaffarCommonDependency
                          ],
    include_directories : include_directories(['source']),
    link_args           : [ '-lncurses' ],
  )

  baseMGBAPlayer = executable('baseMGBAPlayer',
    'source/player.cpp',
    cpp_args            : [ commonCompileArgs, '-DNCURSES' ],
    dependencies        : [ baseMGBADependency,
                            jaffarCommonDependency,
                          ],
    include_directories : include_directories(['source']),
    link_args           : [ '-lncurses' ],
  )

endif

# Building tester tool

newMGBATester = executable('newMGBATester',
  'source/tester.cpp',
  cpp_args            : [ commonCompileArgs ], 
  dependencies        : [ newMGBADependency, jaffarCommonDependency ],
)

# Building tester tool for the original emulator

baseMGBATester = executable('baseMGBATester',
  'source/tester.cpp',
  cpp_args            : [ commonCompileArgs ],
  dependencies        : [ baseMGBADependency, jaffarCommonDependency ],
)

# Building tests
subdir('tests')

endif # If not subproject
