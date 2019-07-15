project('PlaylistMaker_borealis', ['c', 'cpp'],
    version: '1.0.0',
    default_options: [ 'buildtype=release', 'strip=true', 'b_ndebug=if-release', 'cpp_std=c++1z' ],
)

add_global_arguments('-DWINDOW_NAME="PlaylistMaker Borealis"', language : 'cpp')

subdir('borealis/library')

example_files = files(
    'src/PMMainBorealis.cpp',
    'src/Playlist.cpp',
    'src/Playlist.cpp',
    'src/easylogging++.cpp'
)

PlaylistMaker_borealis = executable(
    'PlaylistMaker_borealis',
    [ example_files, borealis_files ],
    dependencies : borealis_dependencies,
    install: true,
    include_directories: [ borealis_include, include_directories('include'), include_directories('CRCpp/inc'), include_directories('../retroarch/libretro-db')],
    cpp_args: [ '-g', '-O2' ]
)
