import os
import re
from tip_scripts.run_cl_process import RunCLProcess

class ValidationBase(object):

    def __init__(self, prefix, debug=0):
        self.test_path = None
        self.truth_path = None
        self.test_passed = None
        self.dry_run = False
        self.cl_process = RunCLProcess(debug=debug)
        self.is_translated_data_comp = False
        self.regex_translated_1553_msg_dir = re.compile(".+_1553_translated.+parquet")
        self.regex_raw_1553_dir = re.compile(".+_1553.parquet")
        self.ready_to_validate = False
        self.prefix = prefix

    def set_1553_paths(self, truth_path, test_path, is_translated_data_comp):
        self.is_translated_data_comp = is_translated_data_comp

        self.test_path = test_path
        self.truth_path = truth_path

        if self.test_path is None or self.truth_path is None:
            return False

        if self.is_translated_data_comp:
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

    def do_validation(self, executable_path):
        if not self.ready_to_validate:
            print('ValidationBase.do_validation(): Not ready to validate.')
            return self.test_passed

        self.cl_process.set_executable_path(executable_path)
        self.cl_process.add_dir_path_argument(self.truth_path)
        self.cl_process.add_dir_path_argument(self.test_path)
        self.cl_process.set_stdout_must_contain('Overall -> Pass')
        did_run = self.cl_process.run(self.dry_run, cwd=os.path.dirname(executable_path), print_stdout=False)
        if did_run:
            ret_val = self.cl_process.get_return_value()
            output_success = self.cl_process.have_output_success()
            if ret_val == 0 and output_success:
                self.test_passed = True
            else:
                if ret_val != 0:
                    print('ValidationBase.do_validation(): ret_val = {:d}'.format(ret_val))
                    # Indicate test was not conducted.
                    self.test_passed = None
                elif not output_success:
                    print('ValidationBase.do_validation(): output_success = False'.format(ret_val))
                    self.test_passed = False

        return self.test_passed

    def get_validation_output(self):
        return self.cl_process.get_output()

    def __repr__(self):
        r = '{:s}\ntruth: {:s}\ntest: {:s}'.format(self.prefix,
                                                   str(self.truth_path),
                                                   str(self.test_path))
        return r






