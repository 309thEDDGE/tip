import os
import sys

if len(sys.argv) < 3:
    print('Not enough args')
    sys.exit(0)

script_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../parse_and_translate.py'))

truth_dir = sys.argv[1] # Directory in which ch10 files and ICDs reside
test_dir = sys.argv[2] # Directory into which generated raw/translated data are placed

ch10_files_and_icds = {}


for ch10,icd in ch10_files_and_icds.items():
    call_list = ['python', script_path, os.path.join(truth_dir, ch10),
                 os.path.join(truth_dir, icd), '-o', test_dir]
    call_string = ' '.join(call_list)
    print(call_string)
    os.system(call_string)