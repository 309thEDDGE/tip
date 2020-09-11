import os
import sys
import datetime
import argparse
script_path = os.path.dirname(os.path.abspath(os.path.join(os.path.realpath(__file__), '../..')))
sys.path.append(script_path)

from tip_scripts.pqpqvalidation.pqpq_raw_validation import PqPqRawValidation
from tip_scripts.pqpqvalidation.pqpq_translated_data_validation import PqPqTranslatedDataValidation
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

        self.truth_set_dir = truth_set_dir
        self.test_set_dir = test_set_dir
        self.log_file_path = log_file_path
        self.video = video
        self.save_stdout = False
        self.raw_validation_dict = {}
        self.transl_validation_dict = {}
        self.total_validation_dict = {}
        self.validation_results_dict = {}
        self.exec_path = 'bin/pqcompare.exe'
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

    def _regenerate_test_set(self):

        print('Regenerating test set\n')

        #Duration: 87 sec

        script_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../parse_and_translate.py'))

        truth_dir = self.truth_set_dir # Directory in which ch10 files and ICDs reside
        test_dir = self.test_set_dir # Directory into which generated raw/translated data are placed

        # read in chapter ten names and associated ICDs from a provided csv
        f = open(self.csv_path, "r")
        lines = f.readlines()

        ch10_files_and_icds = {}
        for line in lines:
            temp = line.split(',')
            ch10_files_and_icds[temp[0].strip()] = temp[1].strip() # strip leading and trailing white space

        for ch10,icd in ch10_files_and_icds.items():
            if self.video:
                 call_list = ['python', script_path, os.path.join(truth_dir, ch10),
                         os.path.join(truth_dir, icd), '-o', test_dir, '--video', '--no-ts']
            else:
                call_list = ['python', script_path, os.path.join(truth_dir, ch10),
                        os.path.join(truth_dir, icd), '-o', test_dir]
            call_string = ' '.join(call_list)
            print(call_string)
            start_time = time.time()
            os.system(call_string)
            self.run_times[ch10] = time.time() - start_time


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
        if self.run_tip:
            self._regenerate_test_set()
        self._open_log()
        self._create_validation_objects()
        self._match_raw_and_translated_comparison_objects()
        self._validate_objects_grouped_by_originating_ch10()
        self._assemble_validation_stats()
        self._present_stats()

    def _present_stats(self):

        for ch10name in self.validation_results_dict.keys():

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

        msg = '\nAll validation set result: {:s}'.format(self.get_validation_result_string(self.validation_results_dict[ch10name]['all_ch10']))
        print(msg)
        self.print(msg)

        print('\nTIP run time stats:')
        self.print('\nTIP run time stats:')
        for key, value in self.run_times.items():
            print('{}: {} seconds'.format(key,round(value,2)))
            self.print('{}: {} seconds'.format(key,round(value,2)))

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
        for ch10name in self.total_validation_dict.keys():
            raw_pass = True
            single_ch10_pass = True
            all_transl_pass = True

            raw_validation_obj = self.total_validation_dict[ch10name]['raw']
            raw_pass = raw_validation_obj.test_passed
            if raw_pass == False or raw_pass is None:
                if all_ch10_pass != False:
                    all_ch10_pass = raw_pass
                if single_ch10_pass != False:
                    single_ch10_pass = raw_pass

            self.validation_results_dict[ch10name] = {'ch10': None, 'raw':raw_pass , 'translated': {}, 'all_translated': None, 'all_ch10': None}

            transl_validation_obj_list = self.total_validation_dict[ch10name]['translated']
            if transl_validation_obj_list is not None:
                for obj in transl_validation_obj_list:
                    transl_pass = True
                    base_name = os.path.basename(obj.truth_path)

                    transl_pass = obj.test_passed
                    if transl_pass == False or transl_pass is None:
                        if all_ch10_pass != False:
                            all_ch10_pass = transl_pass
                        if single_ch10_pass != False:
                            single_ch10_pass = transl_pass
                        if all_transl_pass != False:
                            all_transl_pass = transl_pass

                    self.validation_results_dict[ch10name]['translated'][base_name] = transl_pass

            self.validation_results_dict[ch10name]['all_translated'] = all_transl_pass
            self.validation_results_dict[ch10name]['ch10'] = single_ch10_pass

        self.validation_results_dict[ch10name]['all_ch10'] = all_ch10_pass


    def _validate_objects_grouped_by_originating_ch10(self):

        for ch10name in self.total_validation_dict.keys():
            self.print('\nValidating Ch10 with (presumed) name: {:s}'.format(ch10name))
            print('\n---Validating Ch10 with (presumed) name: {:s}---\n\n'.format(ch10name))

            raw_validation_obj = self.total_validation_dict[ch10name]['raw']
            self.print('\n-- Raw Comparison --\n')
            info = '\n' + str(raw_validation_obj)
            self.print(info)
            print(info)
            result = raw_validation_obj.validate()
            self.print('Validated: {}'.format(result))

            # Get stderr/stdout as necessary and add to log.
            if self.save_stdout:
                stdout, stderr = raw_validation_obj.get_validation_output()
                self.print('\nstdout:')
                self.print(stdout)
                self.print('\nstderr:')
                self.print(stderr)

            transl_validation_obj_list = self.total_validation_dict[ch10name]['translated']
            self.print('\n--Translation Comparison--\n')
            if transl_validation_obj_list is not None:
                for obj in transl_validation_obj_list:

                    info = '\n' + str(obj)
                    self.print(info)
                    print(info)
                    result = obj.validate()
                    self.print('Validated: {}'.format(result))

                    if self.save_stdout:
                        stdout, stderr = obj.get_validation_output()
                        self.print('\nstdout:')
                        self.print(stdout)
                        self.print('\nstderr:')
                        self.print(stderr)


    def _match_raw_and_translated_comparison_objects(self):

        transl_keys = self.transl_validation_dict.keys()
        transl_obj_found = False
        for rawk in self.raw_validation_dict.keys():
            transl_obj_found = False

            # Create the base name common to both raw and translated dirs.
            base_name = rawk[:rawk.find('_1553.parquet')]
            for transk in transl_keys:
                if transk.find(base_name) > -1:
                    self.total_validation_dict[base_name + '.ch10'] = {'raw': self.raw_validation_dict[rawk],
                                                                       'translated': self.transl_validation_dict[transk]}
                    transl_obj_found = True
                    break

            if not transl_obj_found:
                self.total_validation_dict[base_name + '.ch10'] = {'raw': self.raw_validation_dict[rawk],
                                                                       'translated': None}

        #self.print('\nComplete validation set: ' + str(self.total_validation_dict))
        if len(self.total_validation_dict) == 0:
            self.print('Zero entries in valdidation set. Exiting')
            sys.exit(0)

    def _get_paths(self, input_path):

        paths_dict = {}
        if not os.path.isdir(input_path):
            return paths_dict

        dir_contents = os.listdir(input_path)
        #dir_contents = [os.path.join(input_path, x) for x in dir_contents]
        #print(dir_contents)

        paths_dict['raw'] = set()
        paths_dict['translated'] = set()
        for d in dir_contents:
            if d.find('_1553_translated') > 0:
                paths_dict['translated'].add(d)
            elif d.find('_1553.parquet') > 0:
                paths_dict['raw'].add(d)

        # Convert the sets to sorted lists such that 
        # the order of ch10s processed in e2e validation
        # remains the same from run to run.
        paths_dict['raw'] = sorted(list(paths_dict['raw']))
        paths_dict['translated'] = sorted(list(paths_dict['translated']))
        return paths_dict

    def _create_validation_objects(self):

        self.print('truth base dir: ' + self.truth_set_dir)
        self.print('test base dir: ' + self.test_set_dir)

        test_dirs_dict = self._get_paths(self.test_set_dir)
        truth_dirs_dict = self._get_paths(self.truth_set_dir)
        self.print('\nTruth set paths:')
        self.print(str(truth_dirs_dict))
        self.print('\nTest set paths:')
        self.print(str(test_dirs_dict))

        if len(test_dirs_dict) > 0 and (len(test_dirs_dict) == len(truth_dirs_dict)):
            
            # Raw dirs d = name of raw parquet directory
            for d in truth_dirs_dict['raw']:
                if d in test_dirs_dict['raw']:
                    self.raw_validation_dict[d] = PqPqRawValidation(
                        os.path.join(self.truth_set_dir, d),
                        os.path.join(self.test_set_dir, d),
                        self.exec_path)
                else:
                    self.raw_validation_dict[d] = PqPqRawValidation(
                        os.path.join(self.truth_set_dir, d),
                        None,
                        self.exec_path)
                    self.missing_raw_test_paths.append(d)
                    self.print('No matching raw TEST path for TRUTH path {:s}'.format(d))

            # Translated dirs d = name of translated parquet directory
            for d in truth_dirs_dict['translated']:
                self._create_translated_validation_objects(d)
                if not (d in test_dirs_dict['translated']):
                    self.missing_transl_test_paths.append(d)
                    self.print('No matching translated TEST path for TRUTH path {:s}'.format(d))

        else:
            self.print('Count of directories in TRUTH set ({:d}) and TEST set ({:d}) not equal. Exiting.'.format(len(truth_dirs_dict),
                                                                                                                 len(test_dirs_dict)))
            sys.exit(0)

    def _create_translated_validation_objects(self, truth_dir_name):

        self.transl_validation_dict[truth_dir_name] = []
        truth_path = os.path.join(self.truth_set_dir, truth_dir_name)
        truth_dir_listing = os.listdir(truth_path)
        test_path = os.path.join(self.test_set_dir, truth_dir_name)
        test_dir_listing = ""        

        if len(truth_dir_listing) == 0:
            self.missing_transl_truth_paths.append(truth_dir_name)
            self.print('No files in translated TRUTH path {:s}'.format(truth_dir_name))
            return

        if os.path.isdir(test_path):
            test_dir_listing = os.listdir(test_path)       
            for msg_pq_dir in truth_dir_listing:
                msg_parquet_path = os.path.join(truth_path, msg_pq_dir)
                if len(os.listdir(msg_parquet_path)) > 0:
                    if msg_pq_dir in test_dir_listing:
                        test_msg_parquet_path = os.path.join(test_path, msg_pq_dir)
                        if len(os.listdir(test_msg_parquet_path)) > 0:
                            self.transl_validation_dict[truth_dir_name].append(PqPqTranslatedDataValidation(
                                msg_parquet_path, 
                                test_msg_parquet_path, 
                                self.exec_path))
                        # Existing test message parquet directory with no parquet files inside
                        else:
                            self.transl_validation_dict[truth_dir_name].append(PqPqTranslatedDataValidation(
                                msg_parquet_path, 
                                None, 
                                self.exec_path))
                            self.print('No files in translated msg TEST path {:s}'.format(test_msg_parquet_path))
                    # Existing truth message parquet directory but missing test message parquet directory
                    else:
                        self.transl_validation_dict[truth_dir_name].append(PqPqTranslatedDataValidation(
                                msg_parquet_path, 
                                None, 
                                self.exec_path))
                        self.missing_transl_msg_test_paths.append(os.path.join(test_path, msg_pq_dir))
                        self.print('Missing translated msg TEST path {:s}'.format(os.path.join(test_path, msg_pq_dir)))
                # Existing truth message parquet directory with no parquet files inside
                else:
                    self.print('No files in translated msg TRUTH path {:s}'.format(msg_parquet_path))
        else:
            for msg_pq_dir in truth_dir_listing:
                msg_parquet_path = os.path.join(truth_path, msg_pq_dir)
                if len(os.listdir(msg_parquet_path)) > 0:
                    self.transl_validation_dict[truth_dir_name].append(PqPqTranslatedDataValidation(
                            msg_parquet_path, 
                            None, 
                            self.exec_path))
                # Existing truth message parquet directory with no parquet files inside
                else:
                    self.print('No files in translated msg TRUTH path {:s}'.format(msg_parquet_path))

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

    e = E2EValidator(sys.argv[1], sys.argv[2], sys.argv[3], log_desc=log_desc, video=args.video)
    e.validate()