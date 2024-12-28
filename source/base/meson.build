# Gambatte Core sources

gambatteSrc =  [

]

gambatteIncludeDirs = [
  '.',
  '..',
  'core',
  'core/include',
  'core/src'
]

gambatteCompileArgs = [

]

# gambatte Core Configuration

 gambatteDependency = declare_dependency(
  compile_args        : [  gambatteCompileArgs ],
  include_directories : include_directories(gambatteIncludeDirs),
  sources             : [ gambatteSrc ],
  dependencies        : [  
                          dependency('sdl2',  required : true)
                        ]
 )
