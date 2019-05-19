dep_glfw3 = dependency('glfw3', version : '>=3.3')
dep_gl   = dependency('gl')
dep_thread = dependency('threads')

executable('example', 'example.cpp',
           include_directories : inc,
           dependencies : [dep_glfw3, dep_gl, dep_thread], link_with : [libnanogui])
