project('PlaylistMaker_borealis', ['c', 'cpp'],
    version: '1.0.0',
    default_options: [ 'buildtype=release', 'strip=true', 'b_ndebug=if-release', 'cpp_std=c++1z' ],
)

add_global_arguments('-DWINDOW_NAME="PlaylistMaker Borealis"', language : 'cpp')

subdir('borealis/library')

PlaylistMaker_files = files(
    'src/PMMainBorealis.cpp',
    'src/Playlist.cpp',
    'src/Playlist.cpp',
    'src/easylogging++.cpp',
    '../retroarch/libretro-common/streams/file_stream.c',
	'../retroarch/libretro-common/compat/compat_strcasestr.c',
	'../retroarch/libretro-common/file/file_path.c',
	'../retroarch/libretro-common/vfs/vfs_implementation.c',
	'../retroarch/libretro-common/encodings/encoding_utf.c',
	'../retroarch/libretro-common/compat/compat_strl.c',
	'../retroarch/libretro-common/compat/fopen_utf8.c',
	'../retroarch/libretro-common/features/features_cpu.c',
	'../retroarch/libretro-db/rmsgpack.c',
	'../retroarch/libretro-db/rmsgpack_dom.c',
	'../retroarch/libretro-db/bintree.c',
	'../retroarch/libretro-db/query.c',
	'../retroarch/libretro-db/libretrodb.c',
	'../retroarch/libretro-common/compat/compat_fnmatch.c',
	'../retroarch/libretro-common/string/stdstring.c',
)

PlaylistMaker_borealis = executable(
    'PlaylistMaker_borealis',
    [ PlaylistMaker_files, borealis_files ],
    dependencies : borealis_dependencies,
    install: true,
    include_directories: [ borealis_include, include_directories('include'), include_directories('CRCpp/inc'), include_directories('../retroarch/libretro-db')],
    cpp_args: [ '-g', '-O2' ]
)
