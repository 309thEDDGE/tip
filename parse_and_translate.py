
import os, sys
import numpy as np
from pathlib import Path
import platform

# Argument parser
import argparse

# Configure Python path. Add the tip/ root dir to the path.
script_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(script_path)

# Import RunCLProcess class. Helps to make system calls.
from tip_scripts.run_cl_process import RunCLProcess

#
# Function below stolen from: https://stackoverflow.com/questions/39086/search-and-replace-a-line-in-a-file-in-python
#

from tempfile import mkstemp
from shutil import move, copymode, copyfile

def replace_conf_parameter(file_path, param_name, replace_with):
    replace_line = param_name + ' = ' + replace_with + '\n'

    #Create temp file
    fh, abs_path = mkstemp()
    with os.fdopen(fh,'w') as new_file:
        with open(file_path) as old_file:
            for line in old_file:
                if line.find('#') != 0 and line.find(param_name) > -1:
                    new_file.write(replace_line)
                else:
                    new_file.write(line)

    #Copy the file permissions from the old file to the new file
    copymode(file_path, abs_path)
    #Remove original file
    os.remove(file_path)
    #Move new file
    move(abs_path, file_path)

#
# end stolen
#

def get_list_of_files_with_suffix(input_path, the_suffix):
    file_paths = []
    for fname in os.listdir(input_path):
            full_fname = os.path.join(input_path, fname)
            if os.path.isfile(full_fname):
                suff = Path(fname).suffix.lower()
                if suff == the_suffix.lower():
                    file_paths.append(full_fname)
    return file_paths

def skip_message(skip_name):
    print('\nSkipping {:s} - Use --overwrite to force'.format(skip_name))

def ow_message(ov_name):
    print('\nOverwriting files with call to {:s}'.format(ov_name))

if __name__ == '__main__':

    # TODO:
    # - Pass overwrite flags to RunCLProcess.run(). Process checks for output and runs as necessary.
    # - Include logging in run_cl_process.py and exec.py.

    # 
    # Environment setup.
    #
    plat = platform.platform()
    active_plat = None
    if plat.find('Windows') > -1:
        active_plat = 'windows'
    elif plat.find('Linux') > -1:
        active_plat = 'linux'
    else:
        print('Platform {:s} not recognized. Exiting'.format(plat))
        sys.exit(0)

    exe_dir = os.path.join(script_path, 'bin')
    conf_dir = os.path.join(script_path, 'conf')
    default_conf_dir = os.path.join(conf_dir, 'default_conf')
    parse_conf = 'parse_conf.yaml'

    if active_plat == 'windows':
        parser_exe_name = 'tip_parse.exe'
        translator_exe_name = 'tip_translate.exe'
        #comet_exe_name = 'cometparse.exe'
        video_extr_exe_name = 'parquet_video_extractor.exe'
    elif active_plat == 'linux':
        parser_exe_name = 'tip_parse'
        translator_exe_name = 'tip_translate'
        #comet_exe_name = 'cometparse'
        video_extr_exe_name = 'parquet_video_extractor'

    rmcomet_parser_exe_name = parser_exe_name
    rmcomet_translator_exe_name = translator_exe_name
    use_parser_exe = ''
    use_translator_exe = ''
    use_parse_conf = ''
    raw_1553_pq_dir = ''

    #
    # Setup command line arguments
    #
    aparse = argparse.ArgumentParser(description='Parse a Ch10 file and generate tables of '
                                                 'translated engineering units')
    aparse.add_argument('ch10_path', metavar='<Ch10 Path>', type=str, 
                        help='Full path to Ch10 file or directory of Ch10 files')

    aparse.add_argument('icd_path', metavar='<ICD Path>', type=str, 
                        help='Full path to 1553 ICD text or yaml file')

    aparse.add_argument('--overwrite', action='store_true', default=False, 
                        help='Overwrite existing output that is specific to the requested Ch10 '
                        'in the local or specified (-o) directory. '
                        'If overwrite is not specified and Ch10-specific output exists in the '
                        'specified directory, the process that produces existing output will '
                        'be skipped. '
                        'Use this option to ensure new output is created.')

    aparse.add_argument('-o', '--out-path', type=str, default=None, 
                        help='Output path for parsed and translated Ch10 Parquet file. '
                        'Default is path of Ch10 file.')

    aparse.add_argument('--video', action='store_true', default=False, 
                        help='Parse video data and extract to transport stream files.')

    aparse.add_argument('--dry-run', action='store_true', 
                        help='Test input arguments. Do not parse or translate.')

    # aparse.add_argument('--legacy-mode', action='store_true', 
                        # help='Parse and translate in non-RMCOMET mode. '
                        # 'Primary use is for developers.')

    # aparse.add_argument('-n', '--n-threads', type=int, default=4, 
                        # help='(Legacy Mode Only) Thread count for translation routine.')

    # aparse.add_argument('-c', '--comet-path', type=str, default=None, 
                        # help='(Legacy Mode Only) Full path to comet flat files. Turns on legacy mode '
                        # 'if not already specified. Required for legacy mode.')

    # aparse.add_argument('-s', '--comet-search-string', type=str, default=None, 
                        # help='(Legacy Mode Only) String must match set of comet flat files. '
                        # 'Required for legacy mode.')

    

    args = aparse.parse_args()

    if args.video:
        rmcomet_parser_exe_name = parser_exe_name + '_video'
    else:
        rmcomet_parser_exe_name = parser_exe_name

    # if args.comet_path is not None:
        # args.legacy_mode = True

    # Copy configuration files from the default directory
    # to the working configuration file directory.
    # Copy *.conf if in legacy mode, otherwise copy *.yaml
    # files. Only copy files if there exists zero files
    # with the given extension in conf.
    conf_suffix = '.yaml'

    existing_conf_files = get_list_of_files_with_suffix(conf_dir, conf_suffix)
    print('Existing configuration files:\n', existing_conf_files)
    if len(existing_conf_files) == 0:
        default_conf_files = get_list_of_files_with_suffix(default_conf_dir, conf_suffix)
        for defconffile in default_conf_files:
            dest_path = os.path.join(conf_dir, os.path.basename(defconffile))
            print('Copying {:s}'.format(defconffile))
            copyfile(defconffile, dest_path)

    # Get list of Ch10 files.
    ch10_file_paths = []
    suff = ''
    if os.path.isfile(args.ch10_path):
        suff = Path(args.ch10_path).suffix
        if suff == '.ch10' or suff == '.Ch10':
            ch10_file_paths.append(args.ch10_path)
        else:
            print('Input file for Ch10 path does not have \'.ch10\' or \'.Ch10\' suffix')
            sys.exit(0)
    elif os.path.isdir(args.ch10_path):
        ch10_file_paths = get_list_of_files_with_suffix(args.ch10_path, '.Ch10')
        print('Ch10 paths in input directory \'{:s}\':'.format(args.ch10_path))
        for fname in ch10_file_paths:
            print(fname)
    else:
        print("Ch10 path \'{:s}\' does not exist".format(args.ch10_path))
        sys.exit(0)

    # if args.legacy_mode:
        # if args.comet_path is None:
            # print('Legacy mode: Must use \'-c\' option. Use \'-h\' flag for more info.')
            # sys.exit(0)
        # if args.comet_search_string is None:
            # print('Legacy mode: Must use \'-s\' option. Use \'-h\' flag for more info.')
            # sys.exit(0)
        # use_parser_exe = parser_exe_name
        # use_translator_exe = translator_exe_name
        # use_parse_conf = legacy_parse_conf
    # else:
    use_parser_exe = rmcomet_parser_exe_name
    use_translator_exe = rmcomet_translator_exe_name
    use_parse_conf = parse_conf

    did_run = False

    #sys.exit(0)
    for ch10path in ch10_file_paths:
        did_run = False

        # Create raw 1553 output dir.
        raw_1553_pq_dir = Path(ch10path)
        raw_video_pq_dir = ''
        TS_path = ''
        if args.out_path is not None:
            raw_1553_pq_dir = str(Path(args.out_path) / Path(raw_1553_pq_dir.with_name(raw_1553_pq_dir.stem + '_1553.parquet').name))
            raw_video_pq_dir = str(Path(args.out_path) / Path(Path(ch10path).stem + '_video.parquet'))
            TS_path = str(Path(args.out_path) / Path(Path(ch10path).stem + '_video_TS'))
        else:
            raw_1553_pq_dir = str(raw_1553_pq_dir.with_name(raw_1553_pq_dir.stem + '_1553.parquet'))
            raw_video_pq_dir = str(Path(ch10path).with_name(Path(ch10path).stem + '_video.parquet'))
            TS_path = str(Path(ch10path).with_name(Path(ch10path).stem + '_video_TS'))
        #print(raw_1553_pq_dir)

        # Create translated data output dir.
        trans_1553_dir = Path(raw_1553_pq_dir)
        trans_1553_dir = str(trans_1553_dir.with_name(trans_1553_dir.stem + '_translated'))
        #print(trans_1553_dir)

        

        # 
        # Set up Parser call.
        #
        parser_exe_path = os.path.join(exe_dir, use_parser_exe)
        #print(parser_exe_path)
        parser_call = RunCLProcess()
        parser_call.set_executable_path(parser_exe_path)

        # Register required output dir.
        parser_call.register_output_dir(raw_1553_pq_dir)
        parser_call.register_output_dir(raw_video_pq_dir)

        # Check if output dir exists.
        parsed_dir_exists = parser_call.output_dirs_exist()
        if parsed_dir_exists:
            parser_call.message_output_dirs_exist()
            if args.overwrite:
               ow_message(parser_exe_name)
               parser_call.remove_existing_output_dirs()
            else:
                skip_message(parser_exe_name)
        if not parsed_dir_exists or args.overwrite:
        
            # Set input Ch10 file path.
            parser_call.add_file_path_argument(ch10path)

            # Set output directory path if present.
            if args.out_path is not None:
                #print('out_path', args.out_path)
                parser_call.add_dir_path_argument(args.out_path)

            # Set stdout confirmation text.
            parser_call.set_stdout_must_contain('Duration: ')

            # Execute parser.
            did_run = parser_call.run(args.dry_run, cwd=exe_dir)
            if not args.dry_run:
                if not did_run:
                    sys.exit(0)
                if parser_call.get_return_value() != 0 or not parser_call.have_output_success():
                    sys.exit(0)

        #
        # Set up Translator call.
        #
        trans_call = RunCLProcess()

        # Set executable path.
        translate_exe_path = os.path.join(exe_dir, use_translator_exe)
        trans_call.set_executable_path(translate_exe_path)

        # Register required output dir.
        trans_call.register_output_dir(trans_1553_dir)

        # Check if translated output dir exists.
        trans_dir_exists = trans_call.output_dirs_exist()
        if trans_dir_exists:
            trans_call.message_output_dirs_exist()
            if args.overwrite:
               ow_message(translator_exe_name)
               trans_call.remove_existing_output_dirs()
            else:
                skip_message(translator_exe_name)
        if not trans_dir_exists or args.overwrite:

            # Set input raw 1553 parquet directory path.
            trans_call.add_dir_path_argument(raw_1553_pq_dir)

            # Set n threads argument. Only relevant in legacy mode.
            # if args.legacy_mode:
                # trans_call.add_command_argument(args.n_threads)

            # Set ICD text file path. Only relevant in non-legacy
            # mode.
            #if not args.legacy_mode:
            trans_call.add_file_path_argument(args.icd_path)

            # Set stdout confirmation text.
            trans_call.set_stdout_must_contain('Duration: ')

            # Execute translator.
            did_run = trans_call.run(args.dry_run, cwd=exe_dir)
            if not args.dry_run:
                if not did_run:
                    sys.exit(0)
                if trans_call.get_return_value() != 0 or not trans_call.have_output_success():
                    sys.exit(0)


        #
        # Set up TS extractor call (if --video)
        #
        if args.video:

            video_extr = RunCLProcess()

            # Set executable path.
            video_extr_exe_path = os.path.join(exe_dir, video_extr_exe_name)
            video_extr.set_executable_path(video_extr_exe_path)

            # Register required output dir.
            video_extr.register_output_dir(TS_path)

            # Check if directory already exists.
            video_extr_dir_exists = video_extr.output_dirs_exist()
            if video_extr_dir_exists:
                video_extr.message_output_dirs_exist()
                if args.overwrite:
                   ow_message(video_extr_exe_name)
                   video_extr.remove_existing_output_dirs()
                else:
                    skip_message(video_extr_exe_name)
            if not video_extr_dir_exists or args.overwrite:

                # Set input raw video parquet directory path.
                video_extr.add_dir_path_argument(raw_video_pq_dir)

                # Set stdout confirmation text.
                video_extr.set_stdout_must_contain('Elapsed Time: ')

                # Execute video extractor.
                did_run = video_extr.run(args.dry_run, cwd=exe_dir)
                if not args.dry_run:
                    if not did_run:
                        sys.exit(0)
                    if video_extr.get_return_value() != 0 or not video_extr.have_output_success():
                        sys.exit(0)

    sys.exit(0)
