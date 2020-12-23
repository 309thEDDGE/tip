import os
import glob
from setuptools import setup, Extension

tip_root_path = os.path.dirname(os.path.abspath(
    os.path.join(os.path.realpath(__file__), '../../..')))

srcdir_name = 'cpp'
bindir_name = 'bin'
libdir_name = 'lib'

ch10dir_name = 'ch10'

srcdir = os.path.join(tip_root_path, srcdir_name)
bindir = os.path.join(tip_root_path, bindir_name)
libdir = os.path.join(bindir, libdir_name)

source_dirs = [os.path.join(srcdir, ch10dir_name, 'main', 'ch10parse.cpp')]
incl_dirs = [os.path.join(srcdir, ch10dir_name, 'include')]
lib_dirs = [libdir]
libs = glob.glob(os.path.join(libdir, '*.lib'))
print('libs:', libs)

runtime_libs = []

macros = [('PARQUET', None), ('ARROW_STATIC', None), ('PARQUET_STATIC', None)]

setup(
    name='tip_translate',
    ext_modules=[Extension('tip_translate',
                           source_dirs,
                           include_dirs=incl_dirs,
                           library_dirs=lib_dirs,
                           libraries=libs,
                           runtime_library_dirs=runtime_libs,
                           define_macros=macros,
                           )],

    )