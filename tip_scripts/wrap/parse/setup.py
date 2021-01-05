import os, sys
import glob
import platform
from pathlib import Path
from setuptools import setup, Extension

tip_root_path = os.path.dirname(os.path.abspath(
    os.path.join(os.path.realpath(__file__), '../../..')))

def get_platform():

    plat = platform.platform()
    active_plat = None
    if plat.find('Windows') > -1:
        active_plat = 'windows'
    elif plat.find('Linux') > -1:
        active_plat = 'linux'
    else:
        print('Platform {:s} not recognized. Exiting.'.format(plat))
        sys.exit(0)
    
    return active_plat

srcdir_name = 'cpp'
bindir_name = 'bin'
libdir_name = 'lib'
depsdir_name = 'deps'

ch10dir_name = 'ch10'
utildir_name = 'util'

srcdir = os.path.join(tip_root_path, srcdir_name)
bindir = os.path.join(tip_root_path, bindir_name)
libdir = os.path.join(bindir, libdir_name)
depsdir = os.path.join(tip_root_path, depsdir_name)

#source_files = [os.path.join(srcdir, ch10dir_name, 'main', 'ch10parse.cpp')]
source_files = ['parse_wrap.cpp']
incl_dirs = [os.path.join(srcdir, ch10dir_name, 'include'),
             os.path.join(srcdir, utildir_name, 'include')]

third_party_incl_dirs = []
third_party_lib_dirs = []
third_party_libs = []
exports = []
runtime_libs = []

lib_dirs = [libdir]
libs = glob.glob(os.path.join(libdir, '*.lib'))
libs = [Path(x).stem for x in libs]

link_args = []
compile_args = []
d_files = []

# Platform-specific
plat = get_platform()
if plat == 'windows':
    incl_dirs.append(r'C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.27.29110\include')
    lib_dirs.append(r'C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.27.29110\lib\x64')
    compile_args.append('/std:c++17')
    third_party_incl_dirs.extend([os.path.join(depsdir, 'yaml-cpp-yaml-cpp-0.6.0', 
                                      'yaml-cpp-yaml-cpp-0.6.0', 'include'),
                                  os.path.join(depsdir, 'libirig106', 'include'),
                                  os.path.join(depsdir, 'arrow_library_dependencies', 'include'),
                                  os.path.join(depsdir, 'libtins', 'include'),
                                  os.path.join(depsdir, 'npcap', 'include'),
                                  ])
    third_party_lib_dirs.extend([os.path.join(depsdir, 'yaml-cpp-yaml-cpp-0.6.0', 
                                      'yaml-cpp-yaml-cpp-0.6.0', 'build', 'Release'),
                                 os.path.join(depsdir, 'libirig106', 'lib'),
                                 os.path.join(depsdir, 'arrow_library_dependencies', 'lib'),
                                 os.path.join(depsdir, 'libtins', 'lib'),
                                 os.path.join(depsdir, 'npcap', 'lib', 'x64'),
                                 ])
    third_party_libs.extend(['libyaml-cppmd',
                              'irig106',
                              'arrow_static', 'zlibstatic', 'liblz4_static', 'zstd_static',
                              'snappy', 'thriftmd', 'brotlicommon-static', 'brotlienc-static',
                              'brotlidec-static', 'double-conversion', 'parquet_static',
                              'libboost_filesystem-vc140-mt-x64-1_67', 
                              'libboost_system-vc140-mt-x64-1_67',
                              'libboost_regex-vc140-mt-x64-1_67',
                              'tins',
                              'Packet', 'wpcap', 'Ws2_32', 'Iphlpapi',
        ])

    # runtime_library_dirs keyword var does not work in windows. Use
    # setup data_files keyword instead.
    #runtime_libs.extend([os.path.join(depsdir, 'npcap', 'lib', 'x64')])

    d_files = [('', [os.path.join(depsdir, 'npcap', 'lib', 'x64', 'Packet.dll'), 
                     os.path.join(depsdir, 'npcap', 'lib', 'x64', 'wpcap.dll')]),
               ]

    #exports.extend(['RunParser'])

elif plat == 'linux':
    print('linux not ready yet. exiting.')
    sys.exit(0)

incl_dirs.extend(third_party_incl_dirs)
lib_dirs.extend(third_party_lib_dirs)
libs.extend(third_party_libs)

print('incl_dirs:', incl_dirs)
print('libs:', libs)



macros = [('PARQUET', None), ('ARROW_STATIC', None), ('PARQUET_STATIC', None),
          ('TINS_STATIC', None)]

setup(
    name='tip_parse',
    data_files=d_files,
    ext_modules=[Extension('tip_parse',
                           source_files,
                           include_dirs=incl_dirs,
                           library_dirs=lib_dirs,
                           libraries=libs,
                           runtime_library_dirs=runtime_libs,
                           define_macros=macros,
                           extra_compile_args=compile_args,
                           extra_link_args=link_args,
                           export_symbols=exports,
                           )],

    )