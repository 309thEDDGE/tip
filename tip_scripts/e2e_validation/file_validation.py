import os
import re
from pathlib import Path
from tip_scripts.run_cl_process import RunCLProcess
from tip_scripts.e2e_validation.validation_base import ValidationBase

class FileValidation(ValidationBase):

    def __init__(self, prefix):
        ValidationBase.__init__(self, prefix)
        self.dry_run = False
        self.cl_process = RunCLProcess(debug=0)
        self.regex_translated_1553_msg_dir = re.compile(".+_1553_translated.+parquet")
        self.regex_raw_1553_dir = re.compile(".+_1553.parquet")

    def do_validation(self, executable_path, output_success_string='Overall -> Pass'):
        if not self.ready_to_validate:
            print('ValidationBase.do_validation(): Not ready to validate.')
            return self.test_passed

        self.cl_process.set_executable_path(executable_path)
        self.cl_process.add_dir_path_argument(self.truth_path)
        self.cl_process.add_dir_path_argument(self.test_path)
        self.cl_process.set_stdout_must_contain(output_success_string)
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
