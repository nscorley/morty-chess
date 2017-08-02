cxx_binary(
  name = 'morty-chess',
  header_namespace = '',
  headers = subdir_glob([
    ('app/include', '**/*.hpp'),
  ]),
  srcs = glob([
    'app/src/**/*.cpp',
  ]),
  compiler_flags = ['-std=c++11', "-Wall", "-xc++"],
)
