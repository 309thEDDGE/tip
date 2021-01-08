import os, sys
import platform

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

def get_root_path():

    tip_root_path = os.path.dirname(os.path.abspath(
        os.path.join(os.path.realpath(__file__), '../../..')))

    return tip_root_path

def get_src_bin_lib_deps_dirs():

    tip_root_path = get_root_path()

    srcdir_name = 'cpp'
    bindir_name = 'bin'
    libdir_name = 'lib'
    depsdir_name = 'deps'

    srcdir = os.path.join(tip_root_path, srcdir_name)
    bindir = os.path.join(tip_root_path, bindir_name)
    libdir = os.path.join(bindir, libdir_name)
    depsdir = os.path.join(tip_root_path, depsdir_name)

    return srcdir, bindir, libdir, depsdir