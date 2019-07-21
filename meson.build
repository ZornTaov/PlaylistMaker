project('PlaylistMaker_borealis', ['c', 'cpp'],
    version: '1.0.0',
    default_options: [ 'buildtype=release', 'strip=true', 'b_ndebug=if-release', 'cpp_std=c++1z' ],
)

add_global_arguments('-DWINDOW_NAME="PlaylistMaker Borealis"', language : 'cpp')

subdir('borealis/library')

PlaylistMaker_files = files(
    'src/PMMainBorealis.cpp',
    'src/Playlist.cpp',
    'src/PMSettings.cpp',
    'lib/libretro-common/streams/file_stream.c',
	'lib/libretro-common/compat/compat_strcasestr.c',
	'lib/libretro-common/file/file_path.c',
	'lib/libretro-common/vfs/vfs_implementation.c',
	'lib/libretro-common/compat/fopen_utf8.c',
	'lib/libretro-db/rmsgpack.c',
	'lib/libretro-db/rmsgpack_dom.c',
	'lib/libretro-db/bintree.c',
	'lib/libretro-db/query.c',
	'lib/libretro-db/libretrodb.c',
	'lib/libretro-common/compat/compat_fnmatch.c',
	'lib/libretro-common/string/stdstring.c'
)

PlaylistMaker_borealis = executable(
    'PlaylistMaker_borealis',
    [ PlaylistMaker_files, borealis_files ],
    dependencies : borealis_dependencies,
    install: true,
    include_directories: [ borealis_include, include_directories('include'), include_directories('CRCpp/inc'), include_directories('lib/libretro-db')],
    cpp_args: [ '-g', '-O2' ]
)
