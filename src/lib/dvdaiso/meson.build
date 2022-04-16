dvdaiso = static_library(
  'dvdaiso',
  'audio_stream.cpp',
  'audio_stream_info.cpp',
  'audio_track.cpp',
  'dvda_block.cpp',
  'dvda_disc.cpp',
  'dvda_filesystem.cpp',
  'dvda_media.cpp',
  'dvda_metabase.cpp',
  'dvda_zone.cpp',
  'log_trunk.cpp',
  'libmlpdec/mlp.c',
  'libmlpdec/mlpdec.c',
  'libmlpdec/mlpdsp.c',
  'libmlpdec/mlp_parser.c',
  'libmlpdec/mlp_util.c',
  'libudf/dvd_input.cpp',
  'libudf/dvd_reader.cpp',
  'libudf/dvd_udf.c',
  include_directories: inc,
  dependencies: [
    gcrypt_dep,
  ],
)

dvdaiso_dep = declare_dependency(
  link_with: dvdaiso,
)
