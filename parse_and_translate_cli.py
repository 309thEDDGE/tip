import os, sys
import time
import argparse
from pathlib import Path
import platform
import json
from tip_scripts.exec import Exec

# Configure Python path. Add the tip/ root dir to the path.
script_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(script_path)

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
    print('\nSkipping {:s} - Remove --no-overwrite to run'.format(skip_name))
	
def skip_null_message(skip_name):
	print(f"\nSkipping {skip_name} - ICD name is \"null\"")

def ow_message(ov_name):
    print('\nOverwriting files with call to {:s}'.format(ov_name))

def create_log_dir(log_path):

    os.makedirs(log_path, exist_ok=True)
    return True

if __name__ == '__main__':

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

    if active_plat == 'windows':
        parser_exe_name = 'tip_parse.exe'
        translator1553_exe_name = 'tip_translate_1553.exe'
        translator429_exe_name = 'tip_translate_arinc429.exe'
        video_extr_exe_name = 'parquet_video_extractor.exe'
    elif active_plat == 'linux':
        parser_exe_name = 'tip_parse'
        translator1553_exe_name = 'tip_translate_1553'
        translator429_exe_name = 'tip_translate_arinc429'
        video_extr_exe_name = 'parquet_video_extractor'

    #
    # Setup command line arguments
    #
    aparse = argparse.ArgumentParser(description='Parse a Ch10 file and generate tables of '
                                                 'translated engineering units -- for use with TIP CLI')
    aparse.add_argument('ch10_path', metavar='<Ch10 Path>', type=str, 
                        help='Full path to Ch10 file or directory of Ch10 files')

    aparse.add_argument('--dts1553_path', metavar='<1553 ICD Path>', type=str, 
                        help='Full path to 1553 ICD text or yaml file', default=None,
                        required=False)

    aparse.add_argument('--dts429_path', metavar='<ARINC429 ICD Path>', type=str, 
                        help='Full path to ARINC429 ICD text or yaml file', default=None,
                        required=False)

    aparse.add_argument('-o', '--out-path', type=str, default=None, 
                        help='Output path for parsed and translated Ch10 Parquet file. '
                        'Default is path of Ch10 file.')

    # aparse.add_argument('--video', action='store_true', default=False, 
    #                     help='Extract video to transport stream files.')

    aparse.add_argument('--exe-path', type=str, default=None, 
                        help='Set path of tip executables')

    aparse.add_argument('-t', type=int, default=4, 
                        help='Thread count utilization for parse and transle.')

    aparse.add_argument('--no-overwrite', default=False, action='store_true',
        help='Do not parse or translate if output artifact directories exist')


    args = aparse.parse_args()

    if args.exe_path is not None:
        exe_dir = args.exe_path
        print('User exe_dir {:s}'.format(exe_dir))

    # Get list of Ch10 files.
    ch10_file_paths = []
    suff = ''
    if os.path.isfile(args.ch10_path):
        suff = Path(args.ch10_path).suffix.lower()
        if suff == '.ch10':
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

    exec_duration = {}
    did_run = False

    video_ext = 'VIDEO_DATA_F0'
    arinc_ext = 'ARINC429_F0'
    mil1553_ext = 'MILSTD1553_F1'

    for ch10path in ch10_file_paths:

        exec_duration[ch10path] = {'raw1553': None, 'transl1553': None, 'transl429': None}
        did_run = False

        # Create raw 1553 output dir.
        raw_1553_pq_dir = Path(ch10path)
        raw_video_pq_dir = ''
        TS_path = ''
        log_path = ''
        if args.out_path is not None:
            raw_1553_pq_dir = str(Path(args.out_path) / Path(raw_1553_pq_dir.with_name(raw_1553_pq_dir.stem + '_' + mil1553_ext + '.parquet').name))
            raw_video_pq_dir = str(Path(args.out_path) / Path(Path(ch10path).stem + '_' + video_ext + '.parquet'))
            raw_429_pq_dir = str(Path(args.out_path) / Path(Path(ch10path).stem + '_' + arinc_ext + '.parquet'))
            TS_path = str(Path(args.out_path) / Path(Path(ch10path).stem + '_video_TS'))
            log_path = str(Path(args.out_path) / Path('logs'))
        else:
            raw_1553_pq_dir = str(raw_1553_pq_dir.with_name(raw_1553_pq_dir.stem + '_' + mil1553_ext + '.parquet'))
            raw_video_pq_dir = str(Path(ch10path).with_name(Path(ch10path).stem + '_' + video_ext + '.parquet'))
            raw_429_pq_dir = str(Path(ch10path).with_name(Path(ch10path).stem + '_' + arinc_ext + '.parquet'))
            TS_path = str(Path(ch10path).with_name(Path(ch10path).stem + '_video_TS'))
            log_path = str(Path(ch10path).parent / Path('logs'))

        if not create_log_dir(log_path):
            sys.exit(0)

        # Create translated data output dirs.
        trans_1553_dir = str(Path(raw_1553_pq_dir).with_name(Path(raw_1553_pq_dir).stem + '_translated'))
        trans_429_dir = str(Path(raw_429_pq_dir).with_name(Path(raw_429_pq_dir).stem + '_translated'))

        #
        # Parse
        #
        if args.no_overwrite and os.path.isdir(raw_1553_pq_dir):
            skip_message(raw_1553_pq_dir)
            exec_duration[ch10path]['raw1553'] = None
        else:
            parser_exe_path = os.path.join(exe_dir, parser_exe_name)
            print("parser exe: {:s}".format(parser_exe_path))
                
            # Set stdout confirmation text.
            if args.out_path is None:
                parser_command = [parser_exe_path, ch10path, '-t', str(args.t), '-c', '500',
                    '-l', log_path]
            else:
                parser_command = [parser_exe_path, ch10path, '-o', args.out_path, '-t',
                    str(args.t), '-c', '500', '-l', log_path]

            # Execute parser.
            e = Exec()
            retval = e.exec_list(parser_command)
            if retval != 0:
                print(f"Parser command failed: \"{' '.join(parser_command)}\"")
                sys.exit(0)

            exec_duration[ch10path]['raw1553'] = e.get_exec_time()

        #
        # Translate 1553
        #
        if os.path.isdir(raw_1553_pq_dir) and args.dts1553_path is not None:

            if Path(args.dts1553_path).stem == 'null':
                skip_null_message(translator1553_exe_name)
            elif args.no_overwrite and os.path.isdir(trans_1553_dir):
                skip_message(trans_1553_dir)
                exec_duration[ch10path]['transl1553'] = None
            else:
                # Set executable path.
                translate_exe_path = os.path.join(exe_dir, translator1553_exe_name)

                # Execute translator.
                translator_command = [translate_exe_path, raw_1553_pq_dir, args.dts1553_path, 
                    '-t', str(args.t), '-l', log_path]
                exec_trans = Exec()
                retval = exec_trans.exec_list(translator_command)
                if retval != 0:
                    print(f"Translator command failed: \"{' '.join(translator_command)}\"")
                    sys.exit(0)

                exec_duration[ch10path]['transl1553'] = exec_trans.get_exec_time()

        #
        # Translate ARINC429
        #
        if os.path.isdir(raw_429_pq_dir) and args.dts429_path is not None:

            if Path(args.dts429_path).stem == 'null':
                skip_null_message(translator429_exe_name)
            elif args.no_overwrite and os.path.isdir(trans_429_dir):
                skip_message(trans_429_dir)
                exec_duration[ch10path]['transl429'] = None
            else:
                # Set executable path.
                translate_exe_path = os.path.join(exe_dir, translator429_exe_name)

                # Execute translator.
                translator_command = [translate_exe_path, raw_429_pq_dir, args.dts429_path, 
                    '-t', str(args.t), '-l', log_path]
                exec_trans = Exec()
                retval = exec_trans.exec_list(translator_command)
                if retval != 0:
                    print(f"Translator command failed: \"{' '.join(translator_command)}\"")
                    sys.exit(0)

                exec_duration[ch10path]['transl429'] = exec_trans.get_exec_time()

        #
        # Set up TS extractor call (if --video)
        #
        # if args.video:
        #     if native_python:
        #         video_extr_dir_exists = os.path.isdir(TS_path)
        #         if not video_extr_dir_exists or args.overwrite:
        #             ret = tip.extract_video(raw_video_pq_dir)
        #     else:
        #         video_extr = RunCLProcess()

        #         # Set executable path.
        #         video_extr_exe_path = os.path.join(exe_dir, video_extr_exe_name)
        #         video_extr.set_executable_path(video_extr_exe_path)

        #         # Register required output dir.
        #         video_extr.register_output_dir(TS_path)

        #         # Check if directory already exists.
        #         video_extr_dir_exists = video_extr.output_dirs_exist()
        #         if video_extr_dir_exists:
        #             video_extr.message_output_dirs_exist()
        #             if args.overwrite:
        #                 ow_message(video_extr_exe_name)
        #                 video_extr.remove_existing_output_dirs()
        #             else:
        #                 skip_message(video_extr_exe_name)
        #         if not video_extr_dir_exists or args.overwrite:

        #             # Set input raw video parquet directory path.
        #             video_extr.add_dir_path_argument(raw_video_pq_dir)

        #             # Set stdout confirmation text.
        #             video_extr.set_stdout_must_contain('Elapsed Time: ')

        #             # Execute video extractor.
        #             did_run = video_extr.run(args.dry_run, cwd=exe_dir)
        #             if not args.dry_run:
        #                 if not did_run:
        #                     sys.exit(0)
        #                 if video_extr.get_return_value() != 0 or not video_extr.have_output_success():
        #                     sys.exit(0)

        print('\njson:')
        print('[[{:s}]]'.format(json.dumps(exec_duration)))
        #json.dump(exec_duration, sys.stdout)

    sys.exit(0)
