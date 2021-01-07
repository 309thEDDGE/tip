#
# setup.py for tip package
#

import os, sys
import glob
from pathlib import Path
from setuptools import setup, Extension

#wrap_util_path = os.path.dirname(os.path.abspath(os.path.join(
#    os.path.realpath(__file__), '..')))
#sys.path.append(wrap_util_path)

from wrap_util.wrap_util import *

ch10dir_name = 'ch10'
utildir_name = 'util'
transl1553dir_name = 'translate_1553'
pqctxdir_name = 'parquet_context'
vidextractdir_name = os.path.join('video', 'parquet')

# Get absolute paths to src, binaries, libs and deps dirs.
srcdir, bindir, libdir, depsdir = get_src_bin_lib_deps_dirs()

# Configure relevant source files and include dirs.
source_files1 = ['parse/parse_wrap.cpp']
source_files2 = ['translate/translate_wrap.cpp']
source_files3 = ['extract_video/extract_video_wrap.cpp', 
                 ]
incl_dirs = [os.path.join(srcdir, ch10dir_name, 'include'),
             os.path.join(srcdir, utildir_name, 'include'),
             os.path.join(srcdir, transl1553dir_name, 'include'),
             os.path.join(srcdir, pqctxdir_name, 'include'),
             os.path.join(srcdir, vidextractdir_name, 'include')]

# For now, link all libs instead of choosing only the required libs
# which may be most or all of them.
lib_dirs = [libdir]
libs = glob.glob(os.path.join(libdir, '*.lib'))
libs = [Path(x).stem for x in libs]
#libs.remove('pq_vid_extract')
#libs.insert(0, 'arrow_static')
#libs.insert(1, 'parquet_static')

# Prepare empty lists for third party lib paths and
# other potential arguments to the Extension class.
third_party_incl_dirs = []
third_party_lib_dirs = []
third_party_libs = []
exports = []
runtime_libs = []
link_args = []
compile_args = []
d_files = []

# Platform-specific
plat = get_platform()
if plat == 'windows':
    #incl_dirs.append(r'C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.27.29110\include')
    #lib_dirs.append(r'C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.27.29110\lib\x64')
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

elif plat == 'linux':
    print('linux not ready yet. exiting.')
    sys.exit(0)

# Append third party items to the relevant lists.
incl_dirs.extend(third_party_incl_dirs)
lib_dirs.extend(third_party_lib_dirs)
#libs.extend(third_party_libs)
third_party_libs.extend(libs)
libs = third_party_libs

print('incl_dirs:', incl_dirs)
print('libs:', libs)

# Set macros specific to building TIP
macros = [('PARQUET', None), ('ARROW_STATIC', None), ('PARQUET_STATIC', None),
          ('TINS_STATIC', None)]

extensions = [
              Extension('tip_parse',
                           source_files1,
                           include_dirs=incl_dirs,
                           library_dirs=lib_dirs,
                           libraries=libs,
                           runtime_library_dirs=runtime_libs,
                           define_macros=macros,
                           extra_compile_args=compile_args,
                           extra_link_args=link_args,
                           export_symbols=exports,
                           ),
              Extension('tip_translate',
                           source_files2,
                           include_dirs=incl_dirs,
                           library_dirs=lib_dirs,
                           libraries=libs,
                           runtime_library_dirs=runtime_libs,
                           define_macros=macros,
                           extra_compile_args=compile_args,
                           extra_link_args=link_args,
                           export_symbols=exports,
                           ),
              Extension('tip_video',
                           source_files3,
                           include_dirs=incl_dirs,
                           library_dirs=lib_dirs,
                           libraries=libs,
                           runtime_library_dirs=runtime_libs,
                           define_macros=macros,
                           extra_compile_args=compile_args,
                           extra_link_args=link_args,
                           export_symbols=exports,
                           ),
    ]

setup(
    name='tip',
    version='0.0.1',
    data_files=d_files,
    ext_modules=extensions,
    packages=[],
    py_modules=['tip'],
    )