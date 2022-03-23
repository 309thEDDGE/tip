'''
Null/pass/fail logic:
- if a test can be conducted, doesn't crash, doesn't find that files are missing, etc., then the result shall be pass or fail, otherwise null
- the exception to the first bullet are the cases in which files are not required to be present:
  - video *.parquet files
  - 1553 *.parquet files
- a superset of tests is pass only if every subset is pass
- the presence of a single null value in the subset results in a superset null
- a superset is fail if no nulls are present in subset and at least one fail is present in subset


1. ch10 files in the truth directory and not included in the ch10list are not processed or
mentioned in the report
2. raw/translated/video pq present in the test directory and not included in the ch10list
are ignored and do not interfere with the test nor add information to the log
3. raw/translated/video pq present in the truth directory and not included in the ch10list
are ignored
4. xxxxx extra files generated by a newer version of TIP relative to the truth set do not
interfere with the validation (ex: _ethernet.parquet, additional yaml or txt metadata
files within translated data dirs must be exclusively ignored in the
PqPqTranslated1553DirValidation class)
5. ch10 files included in ch10list but not present in truth are recorded as null and
total pass is null
6. correct and present ch10 files included in ch10list paired with icd which does not
exist also results in null for the ch10 and null total pass
7. translated 1553 msg dirs that exist in test data but contain no pq files for
comparison show null results
8. translated 1553 msg dirs that do not exist in test data are reported as null
9. parse and translate durations are recorded as None if the executable was not run,
either by error or because the output files already exist in the test directory
10. metadata files that are present in truth data and do not exist in test data report as
null, cause a null result for the total ch10
11. metadata files that are present in truth data and exist in test data and do not
compare exactly are reported as failed, unless, as in yaml files, there are sub-level
components which are compared individually, in which case the logic given above applies
12. video pq files are only compared when the -v flag is passed to
end_to_end_validator.py
'''


import os
import sys
import time
import datetime
import platform
import json

######################################################################
#                          YAML File Processing
######################################################################
# YAML processing requires the yaml_compare module, which is local to
# this code base. That module relies on the third-party package DeepDiff.
# If DeepDiff ("pip install 'deepdiff[murmur]'") can't be downloaded due
# network access or software restrictions, set the config option below
# to False.
from tip_scripts.e2e_validation import config
from tip_scripts.e2e_validation.txttxt_validation import TxtTxtValidation
#config.COMPARE_YAML = False
######################################################################

tip_root_path = os.path.dirname(os.path.abspath(os.path.join(os.path.realpath(__file__), '../..')))

from tip_scripts.e2e_validation.pqpq_raw1553_dir_validation import PqPqRaw1553DirValidation
from tip_scripts.e2e_validation.pqpq_translated1553_dir_validation import PqPqTranslated1553DirValidation
from tip_scripts.e2e_validation.pqpq_video_dir_validation import PqPqVideoDirValidation
from tip_scripts.e2e_validation.pqpq_rawARINC429_dir_validation import PqPqRawARINC429DirValidation
from tip_scripts.e2e_validation.pqpq_time_data_validation import PqPqTimeDataValidation
from tip_scripts.exec import Exec

class E2EValidator(object):

    def __init__(self, truth_set_dir, test_set_dir, log_file_path, log_desc=''):

        self.run_tip = False
        self.truth_set_dir = truth_set_dir
        self.test_set_dir = test_set_dir
        self.log_file_path = log_file_path
        self.save_stdout = False
        self.all_validation_obj = {}
        self.duration_data = {}
        self.validation_results_dict = {}
        self.print = print

        self.csv_path = os.path.join(truth_set_dir,'ch10list.csv')
        if not os.path.exists(self.csv_path):
            print('\nInvalid csv path {}, not regenerating test set'.format(self.csv_path))
            self.run_tip = False
        else:
            self.run_tip = True

        self.pqcompare_exec_path = os.path.join(tip_root_path, 'bin', 'pqcompare')
        self.bincompare_exec_path = os.path.join(tip_root_path, 'bin', 'bincompare')
        plat = platform.platform()
        if plat.find('Windows') > -1:
            self.pqcompare_exec_path += '.exe'
            self.bincompare_exec_path += '.exe'

        log_description = ''
        if log_desc != '':
            log_description = '{:s}_'.format(log_desc.replace(' ', '-'))
        time_stamp = str(datetime.datetime.now().strftime("%Y%m%d_%H%M%S"))
        log_base_name = 'e2e_validation_' + log_description + time_stamp + '.txt'
        self.log_name = os.path.join(self.log_file_path, log_base_name)
        self.log_handle = None


    def _read_files_under_test(self):

        # The current version of this validator can handle the case in which
        # either the truth ch10 or the icd specified for translation is not
        # present. Specifically avoid checking for the presence of those
        # files here so that validation objects are created and allowed
        # to fail and to record the results so in turn the complete
        # raw and translated data validation can be recorded at the end of
        # the log.

        self.files_under_test = {}

        # read in chapter ten names and associated ICDs from a provided csv
        f = open(self.csv_path, "r")
        lines = f.readlines()

        for line in lines:
            temp = line.split(',')
            basename = temp[0].rstrip('CHch10')[:-1]
            ch10name = temp[0].strip()
            self.files_under_test[ch10name] = {'icd': temp[1].strip(),
                                                      'basename': basename,
                                                      'raw1553': basename + '_1553.parquet',
                                                      'transl1553': basename + '_1553_translated',
                                                      'rawvideo': basename + '_video.parquet',
                                                      'parsedarinc429f0': basename + '_arinc429.parquet',
                                                      'tmats': basename + '_TMATS.txt',
                                                      'time': basename + '_time_data.parquet'}
            self.all_validation_obj[ch10name] = {}
            self.validation_results_dict[ch10name] = {}
            self.duration_data[ch10name] = {'raw1553': None, 'transl1553': None,
                'parsedarinc429f0': None, }

        #print(self.files_under_test)

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

            if config.exe_path is not None:
                call_list = ['python', script_path, ch10_full_path,
                             icd_full_path, '-o', test_dir, '--exe-path', config.exe_path]
            else:
                call_list = ['python', script_path, ch10_full_path,
                             icd_full_path, '-o', test_dir]
            call_string = ' '.join(call_list)
            print(call_string)
            e = Exec()
            retcode = e.exec_list(call_list, cwd=None, print_stdout=True)

            # Do something if retcode != 0?

            # Find the javascript at the end of stdout, load the javascript
            # and save the data.
            stdout, stderr = e.get_output()
            char_ind = stdout.find('json:')
            if char_ind > 0:
                # +5 to get past json:, +1 to get past the newline
                #json_body = stdout[char_ind+5:]
                relevant_section = stdout[char_ind:]
                json_body = relevant_section[relevant_section.find('[[')+2:relevant_section.find(']]')]
                try:
                    duration_data = json.loads(json_body)
                except json.decoder.JSONDecodeError as e:
                    print('json_body:', json_body)
                    print(e)
                    sys.exit(0)

                # Add duration information to dict.
                # Loaded duration dict must be one: key = ch10 path, val = duration info.
                if len(duration_data) == 1:
                    key = list(duration_data.keys())[0]
                    if len(duration_data[key]) > 0:
                        self.duration_data[ch10] = duration_data[key]
                else:
                    print('Duration data does not have length 1!:\n', duration_data)
                    sys.exit(0)

            else:
                print('!!! json data not found in parse_and_translate.py stdout. Exiting. !!!')
                print('\n\nstdout:\n', stdout)
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

        # Read csv with ch10 and icd pairings
        self._read_files_under_test()

        # Run tip using parse_and_translate.py to generate
        # test set.
        if self.run_tip:
            self._regenerate_test_set()

        # Prepare comparison/validation log file
        self._open_log()

        self.print('truth base dir: ' + self.truth_set_dir)
        self.print('test base dir: ' + self.test_set_dir)

        # Create validation objects for 1553 data
        self._create_raw1553_validation_objects()
        self._create_transl1553_validation_objects()
        self._create_rawvideo_validation_objects()
        self._create_rawarinc429f0_validation_objects()
        self._create_tmats_validation_objects()
        self._create_time_data_validation_objects()

        # Validate all objects
        self._validate_objects()

        # Aggregate and print results to stdout and log
        self._assemble_validation_stats()
        self._present_stats()

    def _present_stats(self):

        print('\n---\n')
        self.print('\n---\n')

        for ch10name in self.files_under_test.keys():

            msg = ('\n'
                   +'_________________________________________________________________________\n'
                   +'Validation results for Ch10: {:s}\n'
                   +'=========================================================================').format(ch10name)
            print(msg)
            self.print(msg)

            ########## 1553 ############
            self.all_validation_obj[ch10name]['raw1553'].print_results(self.print)
            self.all_validation_obj[ch10name]['transl1553'].print_results(self.print)

            ########### video ###########
            self.all_validation_obj[ch10name]['rawvideo'].print_results(self.print)

            ########### arinc429f0 #########
            self.all_validation_obj[ch10name]['parsedarinc429f0'].print_results(self.print)

            ########### TMATS ##########
            print()
            self.all_validation_obj[ch10name]['tmats'].print_result(self.print)

            ########### time ##########
            print()
            self.all_validation_obj[ch10name]['time'].print_result(self.print)

            ########### super set ##########
            msg = '\nTotal Ch10 result: {:s}'.format(self.get_validation_result_string(self.validation_results_dict[ch10name]['ch10']))
            print(msg)
            self.print(msg)

            msg = ('_________________________________________________________________________\n')
            print(msg)
            self.print(msg)

        msg = 'Total parsed 1553 data: {:s}'.format(self.get_validation_result_string(self.validation_results_dict['all_raw1553_pass']))
        print(msg)
        self.print(msg)

        msg = 'Total translated 1553 data: {:s}'.format(self.get_validation_result_string(self.validation_results_dict['all_transl1553_pass']))
        print(msg)
        self.print(msg)

        msg = 'Total parsed ARINC429F0 data: {:s}'.format(self.get_validation_result_string(self.validation_results_dict['all_parsedarinc429f0_pass']))
        print(msg)
        self.print(msg)

        msg = 'Total TMATS data: {:s}'.format(self.get_validation_result_string(self.validation_results_dict['all_tmats_pass']))
        print(msg)
        self.print(msg)

        msg = 'Total time data: {:s}'.format(self.get_validation_result_string(self.validation_results_dict['all_time_pass']))
        print(msg)
        self.print(msg)

        msg = 'All validation set result: {:s}'.format(self.get_validation_result_string(self.validation_results_dict['all_ch10']))
        print(msg)
        self.print(msg)

        print('\n---\n')
        self.print('\n---\n')

        print('TIP run time stats:')
        self.print('TIP run time stats:')
        roundval = None
        total_parse_time = 0.0
        total_transl_time = 0.0
        for ch10name in self.duration_data.keys():
            print('\n{:s}:'.format(ch10name))
            self.print('\n{:s}:'.format(ch10name))

            rawdur = self.duration_data[ch10name]['raw1553']
            if rawdur is None:
                roundval = None
                total_parse_time = None
            else:
                roundval = round(rawdur,2)
                if total_parse_time != None:
                    total_parse_time += roundval
            print('Parse: {} seconds'.format(roundval))
            self.print('Parse: {} seconds'.format(roundval))

            transldur = self.duration_data[ch10name]['transl1553']
            if transldur is None:
                roundval = None
                total_transl_time = None
            else:
                roundval = round(transldur,2)
                if total_transl_time != None:
                    total_transl_time += roundval
            print('Translation 1553: {} seconds'.format(roundval))
            self.print('Translation 1553: {} seconds'.format(roundval))

        if total_parse_time is None:
            roundval = None
        else:
            roundval = round(total_parse_time, 2)
        msg = '\nTotal parse time: {} seconds'.format(roundval)
        print(msg)
        self.print(msg)

        if total_transl_time is None:
            roundval = None
        else:
            roundval = round(total_transl_time, 2)
        msg = 'Total translated 1553 time: {} seconds'.format(roundval)
        print(msg)
        self.print(msg)

        if total_parse_time is None or total_transl_time is None:
            roundval = None
        else:
            total_time = total_parse_time + total_transl_time
            roundval = round(total_time, 2)
        msg = 'All validation set time: {} seconds'.format(roundval)
        print(msg)
        self.print(msg)

    def _get_pass_fail_null(self, results_list):

        if results_list.count(None) > 0:
            return None
        elif results_list.count(False) > 0:
            return False
        else:
            return True

    def _assemble_validation_stats(self):

        single_ch10_pass = True
        single_ch10_bulk_transl1553_pass = True
        single_ch10_raw1553_pass = True
        single_ch10_video_pass = True
        single_ch10_parsedarinc429f0_pass = True
        single_ch10_tmats_pass = True
        single_ch10_time_pass = True
        total_stats = {'raw1553': [], 'transl1553': [], 'ch10': [],
            'parsedarinc429f0': [], 'tmats': [], 'time': []}
        for ch10name in self.files_under_test.keys():

            self.validation_results_dict[ch10name] = {'ch10': None, 'raw1553': None,
                                                      'translated1553': None,
                                                      'parsedarinc429f0': None, 'tmats': None,
                                                      'time': None}

            ########## 1553 ############
            single_ch10_raw1553_pass = self.all_validation_obj[ch10name]['raw1553'].get_test_result()
            single_ch10_bulk_transl1553_pass = self.all_validation_obj[ch10name]['transl1553'].get_test_result()

            self.validation_results_dict[ch10name]['raw1553'] = single_ch10_raw1553_pass
            self.validation_results_dict[ch10name]['translated1553'] = single_ch10_bulk_transl1553_pass

            total_stats['raw1553'].append(single_ch10_raw1553_pass)
            total_stats['transl1553'].append(single_ch10_bulk_transl1553_pass)

            ########### ARINC429F0 ###########
            single_ch10_parsedarinc429f0_pass = self.all_validation_obj[ch10name]['parsedarinc429f0'].get_test_result()
            self.validation_results_dict[ch10name]['parsedarinc429f0'] = single_ch10_parsedarinc429f0_pass
            total_stats['parsedarinc429f0'].append(single_ch10_parsedarinc429f0_pass)

            ########### video ###########
            single_ch10_video_pass = self.all_validation_obj[ch10name]['rawvideo'].get_test_result()

            ########### tmats ###########
            single_ch10_tmats_pass = self.all_validation_obj[ch10name]['tmats'].get_test_result() 
            total_stats['tmats'].append(single_ch10_tmats_pass)

            ########### time ###########
            single_ch10_time_pass = self.all_validation_obj[ch10name]['time'].get_test_result() 
            total_stats['time'].append(single_ch10_time_pass)


            ########### super set ##########

            if (single_ch10_raw1553_pass == True and single_ch10_bulk_transl1553_pass == True
                and single_ch10_video_pass == True and single_ch10_parsedarinc429f0_pass == True
                and single_ch10_tmats_pass == True and single_ch10_time_pass == True):
                single_ch10_pass = True
            elif (single_ch10_raw1553_pass is None or single_ch10_bulk_transl1553_pass is None
                  or single_ch10_video_pass is None or single_ch10_parsedarinc429f0_pass is None
                  or single_ch10_tmats_pass is None or single_ch10_time_pass is None):
                single_ch10_pass = None
            else:
                single_ch10_pass = False

            self.validation_results_dict[ch10name]['ch10'] = single_ch10_pass
            total_stats['ch10'].append(single_ch10_pass)

        self.validation_results_dict['all_ch10'] = self._get_pass_fail_null(total_stats['ch10'])
        self.validation_results_dict['all_raw1553_pass'] = self._get_pass_fail_null(total_stats['raw1553'])
        self.validation_results_dict['all_transl1553_pass'] = self._get_pass_fail_null(total_stats['transl1553'])
        self.validation_results_dict['all_parsedarinc429f0_pass'] = self._get_pass_fail_null(total_stats['parsedarinc429f0'])
        self.validation_results_dict['all_tmats_pass'] = self._get_pass_fail_null(total_stats['tmats'])
        self.validation_results_dict['all_time_pass'] = self._get_pass_fail_null(total_stats['time'])


    def _validate_objects(self):

        for ch10name,d in self.files_under_test.items():
            msg = '\n----- Validating Ch10: {:s} -----'.format(ch10name)
            self.print(msg)
            print(msg)

            ################################
            #            1553
            ################################

            ############# Raw ##############
            raw1553_validation_obj = self.all_validation_obj[ch10name]['raw1553']
            self.print('\n-- Parsed 1553F1 Comparison --\n')
            raw1553_validation_obj.validate_dir(self.print)

            ############# Translated ##############
            transl1553_validation_obj = self.all_validation_obj[ch10name]['transl1553']
            self.print('\n-- Translation 1553F1 Comparison --\n')
            transl1553_validation_obj.validate_dir(self.print)

            ############ TMATS ################
            tmats_validation_obj = self.all_validation_obj[ch10name]['tmats']
            self.print('\n-- TMATS Comparison --\n')
            tmats_validation_obj.validate(self.print)

            ############ time ################
            time_validation_obj = self.all_validation_obj[ch10name]['time']
            self.print('\n-- Time Data Comparison --\n')
            time_validation_obj.validate(self.print)

            ################################
            #            video
            ################################
            video_validation_obj = self.all_validation_obj[ch10name]['rawvideo']
            self.print('\n-- Raw VideoF0 Comparison --\n')
            video_validation_obj.validate_dir(self.print)

            ################################
            #            ARINC429F0
            ################################

            ############# Parsed ##############
            parsedarinc429f0_validation_obj = self.all_validation_obj[ch10name]['parsedarinc429f0']
            self.print('\n-- Parsed ARINC429F0 Comparison --\n')
            parsedarinc429f0_validation_obj.validate_dir(self.print)

            ############# Translated ##############
            # translarinc429f0_validation_obj = self.all_validation_obj[ch10name]['translarinc429f0']
            # self.print('\n-- Translation ARINC429F0 Comparison --\n')
            # translarinc429f0_validation_obj.validate_dir(self.print)



    def _create_raw1553_validation_objects(self):
        print("\n-- Create parsed 1553F1 validation objects --\n")
        for ch10name,d in self.files_under_test.items():
            rawname = d['raw1553']
            self.all_validation_obj[ch10name]['raw1553'] = PqPqRaw1553DirValidation(
                os.path.join(self.truth_set_dir, rawname),
                os.path.join(self.test_set_dir, rawname),
                self.pqcompare_exec_path,
                self.bincompare_exec_path)

    def _create_rawarinc429f0_validation_objects(self):
        print("\n-- Create parsed ARINC429F0 validation objects --\n")
        for ch10name,d in self.files_under_test.items():
            rawname = d['parsedarinc429f0']
            self.all_validation_obj[ch10name]['parsedarinc429f0'] = PqPqRawARINC429DirValidation(
                os.path.join(self.truth_set_dir, rawname),
                os.path.join(self.test_set_dir, rawname),
                self.pqcompare_exec_path, self.bincompare_exec_path)

    def _create_transl1553_validation_objects(self):
        print("\n-- Create translated 1553F1 validation objects --\n")
        for ch10name,d in self.files_under_test.items():
            translname = d['transl1553']
            self.all_validation_obj[ch10name]['transl1553'] = PqPqTranslated1553DirValidation(
                os.path.join(self.truth_set_dir, translname),
                os.path.join(self.test_set_dir, translname),
                self.pqcompare_exec_path, self.bincompare_exec_path)

    def _create_rawvideo_validation_objects(self):
        print("\n-- Create parsed VIDEOF0 validation objects --\n")
        for ch10name,d in self.files_under_test.items():
            videoname = d['rawvideo']
            self.all_validation_obj[ch10name]['rawvideo'] = PqPqVideoDirValidation(
                os.path.join(self.truth_set_dir, videoname),
                os.path.join(self.test_set_dir, videoname),
                self.pqcompare_exec_path, self.bincompare_exec_path)

    def _create_tmats_validation_objects(self):
        print("\n-- Create TMATS validation objects --\n")
        for ch10name,d in self.files_under_test.items():
            tmatsname = d['tmats']
            self.all_validation_obj[ch10name]['tmats'] = TxtTxtValidation(
                os.path.join(self.truth_set_dir, tmatsname),
                os.path.join(self.test_set_dir, tmatsname),
                self.bincompare_exec_path)

    def _create_time_data_validation_objects(self):
        print("\n-- Create Time Data validation objects --\n")
        for ch10name,d in self.files_under_test.items():
            timename = d['time']
            self.all_validation_obj[ch10name]['time'] = PqPqTimeDataValidation(
                os.path.join(self.truth_set_dir, timename),
                os.path.join(self.test_set_dir, timename),
                self.pqcompare_exec_path)



    def __del__(self):
        if self.log_handle is not None:
            self.log_handle.close()
