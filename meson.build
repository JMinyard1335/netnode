project('netnode', 'c')

cc = meson.get_compiler('c')

gensio_lib = cc.find_library('gensio', dirs: '/home/jachin/install/lib/')
gensioosh_lib = cc.find_library('gensioosh', dirs: '/home/jachin/install/lib/')
gensioglib_lib = cc.find_library('gensioglib', dirs: '/home/jachin/install/lib/')
gensio_dep = declare_dependency(
  include_directories: include_directories('/home/jachin/install/include'),
  dependencies: [gensio_lib, gensioosh_lib, gensioglib_lib]
)


node_includes = include_directories('include')
netnode = library('netnode', 'src/node.c',
                  include_directories: node_includes,
                  dependencies: [gensio_dep],
                  version: '0.0.1', soversion: '1')

executable('node', 'main.c',
           include_directories: [node_includes],
           link_with: [netnode])
