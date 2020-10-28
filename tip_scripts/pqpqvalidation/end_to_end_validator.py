import os
import sys
import datetime
import argparse
import platform
import json

script_path = os.path.dirname(os.path.abspath(os.path.join(os.path.realpath(__file__), '../..')))
sys.path.append(script_path)

from tip_scripts.pqpqvalidation.pqpq_raw_validation import PqPqRawValidation
from tip_scripts.pqpqvalidation.pqpq_translated_data_validation import PqPqTranslatedDataValidation
from tip_scripts.pqpqvalidation.pqpq_translated_data_dir_validation import PqPqTranslatedDataDirValidation
from tip_scripts.exec import Exec
import time

class E2EValidator(object):

    def __init__(self, truth_set_dir, test_set_dir, log_file_path, log_desc='', video=False):
        self.run_tip = False
        self.csv_path = os.path.join(truth_set_dir,'ch10list.csv')
        if not os.path.exists(self.csv_path):
            print('\nInvalid csv path {}, not regenerating test set'.format(self.csv_path))
            self.run_tip = False
        else:
            self.run_tip = True 

        self.exec_path = os.path.join(script_path, 'bin', 'pqcompare')
        plat = platform.platform()
        if plat.find('Windows') > -1:
            self.exec_path += '.exe'
        
        self.truth_set_dir = truth_set_dir
        self.test_set_dir = test_set_dir
        self.log_file_path = log_file_path
        self.video = video
        self.save_stdout = False
        self.raw_validation_dict = {}
        self.transl_validation_dict = {}
        self.all_validation_obj = {}
        self.duration_data = {}

        # Dictionary for holding validation objects for metadata, yaml, text, or
        # other non-Parquet files that are found in a translated data directory
        # (currently only 1553 translated data).
        # Validation of non-Parquet files has not been implemented.
        self.transl_misc_validation_dict = {}

        # Non-parquet file names to be validated. Not implemented.
        self.transl_misc_validation_fnames = ['_metadata.yaml']

        self.validation_results_dict = {}
        self.print = print
        self.missing_raw_test_paths = []
        self.missing_transl_test_paths = []
        self.missing_transl_truth_paths = []
        self.missing_transl_msg_test_paths = []
        log_description = ''
        if log_desc != '':
            log_description = '{:s}_'.format(log_desc.replace(' ', '-'))
        time_stamp = str(datetime.datetime.now().strftime("%Y%m%d_%H%M%S"))
        log_base_name = 'pqpqvalidation_' + log_description + time_stamp + '.txt'
        self.log_name = os.path.join(self.log_file_path, log_base_name)
        self.log_handle = None
        self.run_times = {}

    def _read_files_under_test(self):

        self.files_under_test = {}

        # read in chapter ten names and associated ICDs from a provided csv
        f = open(self.csv_path, "r")
        lines = f.readlines()
        
        for line in lines:
            temp = line.split(',')
            basename = temp[0].rstrip('.Ch10').rstrip('.ch10')
            ch10name = temp[0].strip()
            self.files_under_test[ch10name] = {'icd': temp[1].strip(), 
                                                      'basename': basename,
                                                      'raw1553': basename + '_1553.parquet',
                                                      'transl1553': basename + '_1553_translated'}
            self.all_validation_obj[ch10name] = {}

        print(self.files_under_test)

    #def _is_raw1553_file_under_test(self, input_fname):
    #    # Input is dir or file.
    #    for testdict in self.files_under_test.values():
    #        if testdict['raw1553'] == input_fname:
    #            return True
    #    return False

    #def _is_translated1553_file_under_test(self, input_fname):
    #    # Input is dir or file.
    #    for testdict in self.files_under_test.values():
    #        if testdict['transl1553'] == input_fname:
    #            return True
    #    return False

    def _regenerate_test_set(self):

        print('\n-- Regenerating test set --\n')

        #Duration: 87 sec

        script_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../parse_and_translate.py'))

        truth_dir = self.truth_set_dir # Directory in which ch10 files and ICDs reside
        test_dir = self.test_set_dir # Directory into which generated raw/translated data are placed

        for ch10,testdict in self.files_under_test.items():

            ch10_full_path = os.path.join(truth_dir, ch10)
            icd_full_path = os.path.join(truth_dir, testdict['icd'])

            # If either the icd or ch10 file do not exist in the truth 
            # directory, then skip the call to parse_and_translate.py.
            if not os.path.isfile(ch10_full_path):
                msg = 'Truth ch10 file does not exist, not generating test data: {:s}\n!'.format(ch10_full_path)
                print(msg)
                continue
            if not os.path.isfile(icd_full_path):
                msg = '\nTruth icd file does not exist, not generating test data: {:s}\n!'.format(icd_full_path)
                print(msg)
                continue

            if self.video:
                 call_list = ['python', script_path, ch10_full_path,
                         icd_full_path, '-o', test_dir, '--video', '--no-ts']
            else:
                call_list = ['python', script_path, ch10_full_path,
                        icd_full_path, '-o', test_dir]
            call_string = ' '.join(call_list)
            print(call_string)
            start_time = time.time()
            #os.system(call_string)
            e = Exec()
            retcode = e.exec_list(call_list, cwd=None, print_stdout=True)
            self.run_times[ch10] = time.time() - start_time

            # Do something if retcode != 0?

            # Find the javascript at the end of stdout, load the javascript
            # and save the data.
            stdout, stderr = e.get_output()
            char_ind = stdout.find('json:')
            if char_ind > 0:
                # +5 to get past json:, +1 to get past the newline
                json_body = stdout[char_ind+5:]
                duration_data = json.loads(json_body)

                # Add duration information to dict.
                # Loaded duration dict must be one: key = ch10 path, val = duration info.
                if len(duration_data) == 1:
                    key = list(duration_data.keys())[0]
                    self.duration_data[ch10] = duration_data[key]
                else:
                    print('Duration data does not have length 1!:\n', duration_data)
                    sys.exit(0)

            else:
                print('!!! json data not found in parse_and_translate.py stdout. Exiting. !!!')
                sys.exit(0)



    def _log_entry(self, entry_str):
        self.log_handle.write(entry_str + '\n')

    def _open_log(self):
        self.log_handle = open(self.log_name, 'w')
        self.print = self._log_entry

    def get_validation_result_string(self, test_passed):
        if test_passed:
            return 'PASS'
        elif test_passed is None:
            return 'NULL'
        elif not test_passed:
            return 'FAIL'
        else:
            return 'BAD RESULT'


    def validate(self):
        self._read_files_under_test()
        if self.run_tip:
            self._regenerate_test_set()
        self._open_log()

        self.print('truth base dir: ' + self.truth_set_dir)
        self.print('test base dir: ' + self.test_set_dir)

        self._create_raw1553_validation_objects()
        self._create_transl1553_validation_objects()
        self._validate_objects()
        self._assemble_validation_stats()
        self._present_stats()

    def _present_stats(self):

        for ch10name in self.files_under_test.keys():

            msg = '\nValidation results for Ch10: {:s}'.format(ch10name)
            print(msg)
            self.print(msg)

            msg = 'Raw: {:s}'.format(self.get_validation_result_string(self.validation_results_dict[ch10name]['raw']))
            print(msg)
            self.print(msg)

            print('Translated data: {:s}'.format(self.get_validation_result_string(self.validation_results_dict[ch10name]['all_translated'])))
            self.print('Translated data:')

            for transl_res_key in self.validation_results_dict[ch10name]['translated'].keys():
                self.print('{:s}: {:s}'.format(transl_res_key, self.get_validation_result_string(self.validation_results_dict[ch10name]['translated'][transl_res_key])))

            msg = 'Total Ch10 result: {:s}'.format(self.get_validation_result_string(self.validation_results_dict[ch10name]['ch10']))
            print(msg)
            self.print(msg)

        msg = '\nAll validation set result: {:s}'.format(self.get_validation_result_string(self.validation_results_dict['all_ch10']))
        print(msg)
        self.print(msg)

        print('\nTIP run time stats:')
        self.print('\nTIP run time stats:')
        roundval = None
        for ch10name in self.duration_data.keys():
            print('\n{:s}:'.format(ch10name))
            self.print('\n{:s}:'.format(ch10name))

            rawdur = self.duration_data[ch10name]['raw']
            if rawdur is None:
                roundval = None
            else:
                roundval = round(rawdur,2)
            print('raw: {} seconds'.format(roundval))
            self.print('raw: {} seconds'.format(roundval))

            transldur = self.duration_data[ch10name]['transl']
            if transldur is None:
                roundval = None
            else:
                roundval = round(transldur,2)
            print('translation: {} seconds'.format(roundval))
            self.print('translation: {} seconds'.format(roundval))

    def _assemble_validation_stats(self):

        '''
        For single_ch10_pass, all_transl_pass and all_ch10_pass:
        If a single instance of False occurs, the value will be set to False forever.
        If one or more None results are retrieved from the validation object, and no
        False results, then the value will be None.
        '''

        all_ch10_pass = True
        single_ch10_pass = True
        transl_pass = True
        raw_pass = True
        all_transl_pass = True
        all_ch10_pass_is_set = False
        for ch10name in self.files_under_test.keys():
            raw_pass = self.all_validation_obj[ch10name]['raw1553'].test_passed
            all_transl_pass = self.all_validation_obj[ch10name]['transl1553'].all_passed
            if raw_pass == True and all_transl_pass == True:
                single_ch10_pass = True
            elif raw_pass is None or all_transl_pass is None:
                single_ch10_pass = None
            else:
                single_ch10_pass = False

            self.validation_results_dict[ch10name] = {'ch10': single_ch10_pass, 'raw': raw_pass, 
                                                      'translated': {}, 'all_translated': all_transl_pass}

            transl_validation_obj_list = self.all_validation_obj[ch10name]['transl1553'].validation_objects
            if transl_validation_obj_list is not None:
                for obj in transl_validation_obj_list:
                    base_name = os.path.basename(obj.truth_path)
                    self.validation_results_dict[ch10name]['translated'][base_name] = obj.test_passed

            # Logic for the entire set:
            # - all individual ch10 pass, set true
            # - individual ch10 status mixed true and None, set None
            # - single false indicates false for set
            if not all_ch10_pass_is_set:
                if single_ch10_pass is None:
                    all_ch10_pass = None
                elif single_ch10_pass == False:
                    all_ch10_pass = False
                    all_ch10_pass_is_set = True

        self.validation_results_dict['all_ch10'] = all_ch10_pass


    def _validate_objects(self):

        for ch10name,d in self.files_under_test.items():
            msg = '\n----- Validating Ch10: {:s} -----'.format(ch10name)
            self.print(msg)
            print(msg)

            raw_validation_obj = self.all_validation_obj[ch10name]['raw1553']
            self.print('\n-- Raw Comparison --\n')
            info = '\n' + str(raw_validation_obj)
            self.print(info)
            print(info)
            rawresult = raw_validation_obj.validate()
            self.print('Validated: {}'.format(rawresult))

            # Get stderr/stdout as necessary and add to log.
            if self.save_stdout:
                stdout, stderr = raw_validation_obj.get_validation_output()
                self.print('\nstdout:')
                self.print(stdout)
                self.print('\nstderr:')
                self.print(stderr)

            transl_validation_obj = self.all_validation_obj[ch10name]['transl1553']
            self.print('\n--Translation Comparison--\n')
            transl_validation_obj.validate(self.print, self.save_stdout)

    def _create_raw1553_validation_objects(self):
        print("\n-- Create raw 1553 validation objects --\n")    
        for ch10name,d in self.files_under_test.items():
            rawname = d['raw1553']
            self.all_validation_obj[ch10name]['raw1553'] = PqPqRawValidation(
                os.path.join(self.truth_set_dir, rawname),
                os.path.join(self.test_set_dir, rawname),
                self.exec_path)

    def _create_transl1553_validation_objects(self):
        print("\n-- Create translated 1553 validation objects --\n")
        for ch10name,d in self.files_under_test.items():
            translname = d['transl1553']
            self.all_validation_obj[ch10name]['transl1553'] = PqPqTranslatedDataDirValidation(
                self.truth_set_dir, self.test_set_dir, translname, self.exec_path)

    def __del__(self):
        if self.log_handle is not None:
            self.log_handle.close()


if __name__ == '__main__':

    #if len(sys.argv) < 4:
    #    print('Not enough args')
    #    sys.exit(0)

    #desc = ''
    #if len(sys.argv) == 5:
    #    desc = sys.argv[4]

    #e = E2EValidator(sys.argv[1], sys.argv[2], sys.argv[3], log_desc=desc, video=False)

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
    aparse.add_argument('-v', '--video', action='store_true', default=False,
                        help='Generate raw video parquet files (validation not implemented)')

    args = aparse.parse_args()
    log_desc = ''
    if args.log_string is not None:
        log_desc = args.log_string

    e = E2EValidator(args.truth_dir, args.test_dir, args.log_dir, log_desc=log_desc, video=args.video)
    e.validate()