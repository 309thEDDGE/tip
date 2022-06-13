import os
import re
from pathlib import Path

class ValidationBase(object):

    def __init__(self, prefix, debug=0):
        self.test_path = None
        self.truth_path = None
        self.test_passed = None
        self.regex_translated_1553_msg_dir = re.compile(".+_MILSTD1553_F1_translated.+parquet")
        self.regex_raw_1553_dir = re.compile(".+_MILSTD1553_F1.parquet")
        self.regex_raw_video_dir = re.compile(".+_VIDEO_DATA_F0.parquet")
        self.regex_parsed_arinc429_dir = re.compile(".+_ARINC429_F0.parquet")
        self.regex_translated_arinc429_dir = re.compile(".+_ARINC429_F0.+parquet")
        self.regex_parsed_timef1_file = re.compile(".+_TIME_DATA_F1.parquet")
        self.ready_to_validate = False
        self.prefix = prefix
        self.truth_dir_exists = None
        self.test_dir_exists = None
        self.truth_file_exists = None
        self.test_file_exists = None

    def get_test_result(self):
        return self.test_passed

    def get_test_result_string(self):
        return self.validation_result_string(self.test_passed)

    def get_test_result_string_from_input(self, input_result):
        return self.validation_result_string(input_result)

    def set_paths(self, truth_path, test_path):

        self.test_path = test_path
        self.truth_path = truth_path

        if self.truth_path is None:
            print('{:s} - {:s} is None!'.format(self.prefix, self.truth_path))
            return False

        if self.test_path is None:
            print('{:s} - {:s} is None!'.format(self.prefix, self.test_path))
            return False

        return True

    def validation_result_string(self, test_passed):
        if test_passed:
            return 'PASS'
        elif test_passed is None:
            return 'NULL'
        elif not test_passed:
            return 'FAIL'
        else:
            return 'BAD RESULT'

    def set_directory_paths(self, truth_path, test_path):

        if not self.set_paths(truth_path, test_path):
            return False

        if not os.path.isdir(self.truth_path):
           print('{:s} - {:s} is not a directory!'.format(self.prefix, self.truth_path))
           self.truth_dir_exists = False
           return False
        self.truth_dir_exists = True

        if not os.path.isdir(self.test_path):
            print('{:s} - {:s} is not a directory!'.format(self.prefix, self.test_path))
            self.test_dir_exists = False
            return False
        self.test_dir_exists = True

        return True

    def set_file_paths(self, truth_path, test_path):

        if not self.set_paths(truth_path, test_path):
            return False

        if not os.path.isfile(self.truth_path):
            print('{:s} - {:s} is not a file!'.format(self.prefix, self.truth_path))
            self.truth_file_exists = False
            return False
        self.truth_file_exists = True

        if not os.path.isfile(self.test_path):
            print('{:s} - {:s} is not a file!'.format(self.prefix, self.test_path))
            self.test_file_exists = False
            return False
        self.test_file_exists = True

        return True

    def set_type_paths(self, truth_path, test_path, type_str):

        if not self.set_paths(truth_path, test_path):
            return False

        if type_str == 'transl1553f1':
            if not self._is_translated_1553_msg_dir(self.truth_path):
                print('{:s} - {:s} is not a translated 1553 msg dir!'.format(self.prefix, self.truth_path))
                self.truth_dir_exists = False
                return False
            self.truth_dir_exists = True
            if not self._is_translated_1553_msg_dir(self.test_path):
                print('{:s} - {:s} is not a translated 1553 msg dir!'.format(self.prefix, self.test_path))
                self.test_dir_exists = False
                return False
            self.test_dir_exists = True

        elif type_str == 'parsed1553f1':
            if not self._is_raw_1553_dir(self.truth_path):
                print('{:s} - {:s} is not a parsed 1553 dir!'.format(self.prefix, self.truth_path))
                self.truth_dir_exists = False
                return False
            self.truth_dir_exists = True
            if not self._is_raw_1553_dir(self.test_path):
                print('{:s} - {:s} is not a parsed 1553 dir!'.format(self.prefix, self.test_path))
                self.test_dir_exists = False
                return False
            self.test_dir_exists = True

        elif type_str == 'parsedvideof0':
            if not self._is_raw_video_dir(self.truth_path):
                print('{:s} - {:s} is not a parsed video dir!'.format(self.prefix, self.truth_path))
                self.truth_dir_exists = False
                return False
            self.truth_dir_exists = True
            if not self._is_raw_video_dir(self.test_path):
                print('{:s} - {:s} is not a parsed video dir!'.format(self.prefix, self.test_path))
                self.test_dir_exists = False
                return False
            self.test_dir_exists = True

        elif type_str == 'parsedarinc429f0':
            if not self._is_parsed_arinc429_dir(self.truth_path):
                print('{:s} - {:s} is not a parsed arinc429 dir!'.format(self.prefix, self.truth_path))
                self.truth_dir_exists = False
                return False
            self.truth_dir_exists = True
            if not self._is_parsed_arinc429_dir(self.test_path):
                print('{:s} - {:s} is not a parsed arinc429 dir!'.format(self.prefix, self.test_path))
                self.test_dir_exists = False
                return False
            self.test_dir_exists = True

        elif type_str == 'transl429f0':
            if not self._is_translated_arinc429_dir(self.truth_path):
                print('{:s} - {:s} is not a translated ARINC429 word dir!'.format(self.prefix, self.truth_path))
                self.truth_dir_exists = False
                return False
            self.truth_dir_exists = True
            if not self._is_translated_arinc429_dir(self.test_path):
                print('{:s} - {:s} is not a translated ARINC429 word dir!'.format(self.prefix, self.test_path))
                self.test_dir_exists = False
                return False
            self.test_dir_exists = True

        elif type_str == 'timef1':
            if not self._is_timef1_file(self.truth_path):
                print('{:s} - {:s} is not a time data file!'.format(self.prefix, self.truth_path))
                self.truth_dir_exists = False
                return False
            self.truth_dir_exists = True
            if not self._is_timef1_file(self.test_path):
                print('{:s} - {:s} is not a time data file!'.format(self.prefix, self.test_path))
                self.test_dir_exists = False
                return False
            self.test_dir_exists = True

        else:
            print('ValidationBase.set_type_paths(): type_str = {:s} not defined!'.format(type_str))
            return False

        return True

    def _is_translated_1553_msg_dir(self, input_path):
        if not os.path.isdir(input_path):
            return False

        if bool(re.match(self.regex_translated_1553_msg_dir, input_path)):
            return True
        return False

    def _is_raw_1553_dir(self, input_path):
        if not os.path.isdir(input_path):
            return False

        if bool(re.match(self.regex_raw_1553_dir, input_path)):
            return True
        return False

    def _is_raw_video_dir(self, input_path):
        if not os.path.isdir(input_path):
            return False

        if bool(re.match(self.regex_raw_video_dir, input_path)):
            return True
        return False

    def _is_parsed_arinc429_dir(self, input_path):
        if not os.path.isdir(input_path):
            return False

        if bool(re.match(self.regex_parsed_arinc429_dir, input_path)):
            return True

    def _is_translated_arinc429_dir(self, input_path):
        if not os.path.isdir(input_path):
            return False

        if bool(re.match(self.regex_translated_arinc429_dir, input_path)):
            return True
        return False

    def _is_timef1_file(self, input_path):
        if not os.path.isfile(input_path):
            return False
        
        if bool(re.match(self.regex_parsed_timef1_file, input_path)):
            return True

        return False

    def directory_has_files_with_extensions(self, dir, exts_list):

        dir_path = Path(dir)
        if not dir_path.is_dir():
            print('directory_has_files_with_extensions(): Input dir \"{:s}\" is '
                  'not a directory'.format(dir))
            return False

        file_list = os.listdir(dir)
        file_exts_list = [Path(f).suffix for f in file_list]
        dot_exts_list = ['.' + ext for ext in exts_list]
        if len(set(file_exts_list).intersection(set(dot_exts_list))) == 0:
            return False
        return True

    def __repr__(self):
        r = '{:s}\ntruth: {:s}\ntest: {:s}'.format(self.prefix,
                                                   str(self.truth_path),
                                                   str(self.test_path))
        return r






