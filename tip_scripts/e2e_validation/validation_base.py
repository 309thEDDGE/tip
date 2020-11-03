import os
import re
from pathlib import Path

class ValidationBase(object):

    def __init__(self, prefix, debug=0):
        self.test_path = None
        self.truth_path = None
        self.test_passed = None
        self.regex_translated_1553_msg_dir = re.compile(".+_1553_translated.+parquet")
        self.regex_raw_1553_dir = re.compile(".+_1553.parquet")
        self.ready_to_validate = False
        self.prefix = prefix

    def set_paths(self, truth_path, test_path):

        self.test_path = test_path
        self.truth_path = truth_path

        if self.test_path is None or self.truth_path is None:
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

    def set_file_paths(self, truth_path, test_path):

        if not self.set_paths(truth_path, test_path):
            return False

        if not os.path.isfile(self.truth_path) or not os.path.isfile(self.test_path):
            return False

        return True

    def set_1553_paths(self, truth_path, test_path, is_translated_data_comp):

        if not self.set_paths(truth_path, test_path):
            return False

        if is_translated_data_comp:
            if not self._is_translated_1553_msg_dir(self.test_path):
                return False
            if not self._is_translated_1553_msg_dir(self.truth_path):
                return False
        else:
            if not self._is_raw_1553_dir(self.test_path):
                return False
            if not self._is_raw_1553_dir(self.truth_path):
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

    def __repr__(self):
        r = '{:s}\ntruth: {:s}\ntest: {:s}'.format(self.prefix,
                                                   str(self.truth_path),
                                                   str(self.test_path))
        return r






