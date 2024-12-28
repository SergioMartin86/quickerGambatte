# quickerGambatte Core sources

quickerGambatteSrc =  [

]

quickerGambatteIncludeDirs = [
  '.',
  '..',
  'core',
  'core/include',
  'core/src'
]

quickerGambatteCompileArgs = [

]

# quickerGambatte Core Configuration

 quickerGambatteDependency = declare_dependency(
  compile_args        : [  quickerGambatteCompileArgs ],
  include_directories : include_directories(quickerGambatteIncludeDirs),
  sources             : [ quickerGambatteSrc ],
  dependencies        : [  
                          dependency('sdl2',  required : true)
                        ]
 )
