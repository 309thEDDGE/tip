import os, sys
import numpy as np
from pathlib import Path
from shutil import move

# Argument parser
import argparse

# Configure Python path. Add the tip/ root dir to the path.
script_path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
sys.path.append(script_path)

# Import RunCLProcess class. Helps to make system calls.
from scripts.run_cl_process import RunCLProcess

if __name__ == '__main__':
    
    # 
    # Environment setup.
    #

    #
    # Setup command line arguments
    #
    aparse = argparse.ArgumentParser(
        description='Generate text file ICD represenation from comet flat files')
    aparse.add_argument('exe_path', metavar='<ICD tool exe path>', 
                        type=str, help='Full path to comet flat files ICD generator')
    aparse.add_argument('input_dir', metavar='<comet flat files dir>', 
                        type=str, help='Full path to directory containing set(s) of comet flat files')
    aparse.add_argument('-o', '--out-path', type=str, default=None, 
                        help='Output path for parsed Ch10 Parquet file')

    args = aparse.parse_args()

    # Confirm that exe_path exists.
    if not os.path.isfile(args.exe_path):
        print('Executable path \'{:s}\' not found'.format(args.exe_path))
        sys.exit(0)

    # Confirm that input directory exists.
    if not os.path.isdir(args.input_dir):
        print('Input directory \'{:s}\' not found'.format(args.input_dir))
        sys.exit(0)

    # Confirm that output directory exists.
    if args.out_path is not None:
        if not os.path.isdir(args.out_path):
            print('Output directory \'{:s}\' not found'.format(args.out_path))
            sys.exit(0)

    #
    # Itemize sets of comet flat files to be translated into ICD text format.
    #

    # Get set of files within input dir that have suffix '.tbl'
    all_flat_files = [x for x in os.listdir(args.input_dir) if Path(x).suffix == '.tbl']
    if len(all_flat_files) == 0:
        print('No files found in \'{:s}\' with .tbl extension'.format(args.input_dir))
        sys.exit(0)

    # Compile a set of the files with '_msg.tbl'.
    msg_flat_files = set([x for x in all_flat_files if x.find('_msg.tbl') > 0])

    # For each *_msg.tbl, extract the string up to and including the underscore. Use
    # string to find matching *_wrd.tbl, *_bit.tbl, and *_term_mux_addr.tbl files.
    required_suffixes = ['wrd.tbl', 'bit.tbl', 'term_mux_addr.tbl'] # in addition to 'msg.tbl'
    found_suffix = []
    suite_file_paths = []
    list_of_suites = []
    search_keys = []
    for msgfile in msg_flat_files:
        prefix = msgfile[:msgfile.find('_')]
        search_keys.append(prefix)
        #suite_file_paths = [os.path.join(args.input_dir, prefix + x) for x in required_suffixes]
        #found_suffix = [os.path.isfile(x) for x in suite_file_paths]
        #if np.sum(np.array(found_suffix)) == len(found_suffix):
        #    list_of_suites.append(suite_file_paths)

    # Loop over each search key and execute the ICD generation process.
    output_file = ''
    dest_path = ''
    icd_suffix = '_ICD.txt'
    for k in sorted(search_keys):
        output_file = os.path.join(args.input_dir, k + icd_suffix)
        if args.out_path is not None:
            dest_path = os.path.join(args.out_path, k + icd_suffix)

        icdgen_call = RunCLProcess()
        icdgen_call.set_executable_path(args.exe_path)
        
        # Set comet flat files file path argument.
        icdgen_call.add_dir_path_argument(args.input_dir)

        # Set search string argument.
        icdgen_call.add_command_argument(k)

        # Set control word argument.
        icdgen_call.add_command_argument('text')

        # Set stdout confirmation text.
        icdgen_call.set_stdout_must_contain('Wrote ICD text file: ')

        # Register required output file.
        icdgen_call.register_output_file(output_file)

        # Execute.
        did_run = icdgen_call.run(False, cwd=None)
        if not did_run:
            break
        if icdgen_call.get_return_value() != 0 or not icdgen_call.have_output_success():
            break

        # If output archive is to be copied to another directory, check if it exists
        # and move it.
        if args.out_path is not None:
            move(output_file, dest_path)

    sys.exit(0)