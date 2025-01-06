project(
    'cjit',
    'c',
    version: run_command('./meson_version.sh', capture:true, check: true).stdout().strip(),
    license: 'GPL3',
     meson_version: '>=1.1',
    default_options: [
        'warning_level=2',
    ],
)

message('Building project version: @0@'.format(meson.project_version()))

# Define the source files

src = files( '../src/cjit.c', '../src/file.c', '../src/elflinker.c',
'../src/main.c', '../src/cwalk.c', '../src/array.c',
'../src/muntar.c', '../src/tinflate.c', '../src/tinfgzip.c')

# Check for tcc command
tcc_exe = find_program('tcc', required: true)
if tcc_exe.found()
  tcc_found = true
endif

# Detect system architecture
arch = host_machine.cpu()

# Construct the path based on architecture
lib_paths = []
if arch == 'x86_64'
  lib_paths += [
    '/usr/lib/x86_64-linux-gnu/tcc/'
  ]
elif arch == 'i686'
  lib_paths += [
    '/usr/lib/i386-linux-gnu/libtcc1.a',
    '/usr/lib/i386-linux-gnu/tcc/libtcc1.a'
  ]
elif arch == 'arm'
  lib_paths += [
    '/usr/lib/arm-linux-gnueabihf/libtcc1.a',
    '/usr/lib/arm-linux-gnueabihf/tcc/libtcc1.a'
  ]
elif arch == 'aarch64'
  lib_paths += [
    '/usr/lib/aarch64-linux-gnu/libtcc1.a',
    '/usr/lib/aarch64-linux-gnu/tcc/libtcc1.a'
  ]
elif arch == 'riscv'
  lib_paths += [
    '/usr/lib/riscv64-linux-gnu/libtcc1.a',
    '/usr/lib/riscv64-linux-gnu/tcc/libtcc1.a'
  ]
# Add other architectures as needed
else
  error('Unsupported architecture: @0@'.format(arch))
endif

libtcc_found = 'no'
# Check if libtcc1.a exists in the constructed paths
foreach path : lib_paths
  libtcc = meson.get_compiler('c').find_library('tcc1',
												dirs : path,
												required : false,
												static : true)
  if libtcc.found()
	libtcc_found = path
	break
  endif
endforeach

# Error out if tcc or libtcc1.a is not found
if not tcc_found or libtcc_found == 'no'
  error('tcc command and libtcc1.a library are required but not found')
else
  message('using system provided @0@libtcc1.a'.format(libtcc_found))
endif



# Define variables
prefix = get_option('prefix')
version = meson.project_version()

# Compiler arguments
c_args = [ '-I../src',
		   '-DSHAREDTCC="@0@libtcc1.a"'.format(libtcc_found),
		   '-DPREFIX="@0@"'.format(prefix),
		   '-DVERSION="@0@"'.format(version), ]

# Build the executable
executable('cjit', src,
  dependencies: [libtcc],
  link_args: ['-ltcc'],
  install: true,
  c_args: c_args)
