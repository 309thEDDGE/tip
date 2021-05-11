import os
import sys
import argparse

tip_root_path = os.path.dirname(os.path.abspath(os.path.join(os.path.realpath(__file__), '../..')))
sys.path.append(tip_root_path)

from tip_scripts.e2e_validation import config

if __name__ == '__main__':

    aparse = argparse.ArgumentParser(description='Do end-to-end validation on a set of Ch10 files')
    aparse.add_argument('truth_dir', metavar='<truth dir. path>', type=str, 
                        help='Full path to directory containing source Ch10 files, '
                        'previously generated parsed and translated output data, '
                        'an ICD to be used during the 1553 translation step for each '
                        'ch10 file and a ch10list.csv file which matches the ch10 name '
                        'to the ICD to be used during translation. ch10list.csv format:\n'
                        '<ch10 name 1> <icd name for ch10 1> # both present in the truth direcotry\n'
                        '<ch10 name 2> <icd name for ch10 2>\n'
                        '...')
    aparse.add_argument('test_dir', metavar='<test dir. path>', type=str, 
                        help='Full path to directory in which freshly generated output data shall be '
                        'placed. TIP parse and translate routines will be applied to each of the ch10 '
                        'files found in the truth directory and placed in the test dir.')
    aparse.add_argument('log_dir', metavar='<log output dir. path>', type=str, 
                         help='Full path to directory in which logs shall be written')
    aparse.add_argument('-l', '--log-string', type=str, default=None,
                        help='Provide a string to be inserted into the log name')
    aparse.add_argument('--no-yaml', action='store_true', default=False, 
                        help='Turn off Yaml dependency import (deepdiff) and do not compare yaml files')

    args = aparse.parse_args()
    log_desc = ''
    if args.log_string is not None:
        log_desc = args.log_string

    # Define the COMPARE_YAML global config prior to importing E2EValidator
    config.COMPARE_YAML = True
    if args.no_yaml:
        config.COMPARE_YAML = False

    from tip_scripts.e2e_validation.end_to_end_validator import E2EValidator
    e = E2EValidator(args.truth_dir, args.test_dir, args.log_dir, log_desc=log_desc)
    e.validate()