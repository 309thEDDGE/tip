import os
from tip_scripts.e2e_validation.file_validation import FileValidation

class TxtTxtValidation(FileValidation):

    def __init__(self, truth_path, test_path, exec_path):
        FileValidation.__init__(self, 'TxtTxtValidation')
        self.ready_to_validate = self.set_file_paths(truth_path, test_path)
        self.exec_path = exec_path

    def validate(self, print_obj):
        info = '\n' + str(self)
        print_obj(info)
        print(info) 
        result = self.do_file_validation(self.exec_path, output_success_string='PASS')
        msg = 'Validated: {}'.format(self.get_test_result_string())
        print_obj(msg)    
        print(msg)

    def print_results(self, print_obj):

        result = 'NULL'
        if self.test_passed == True:
            result = 'PASS'
        elif self.test_passed == False:
            result = 'FAIL'

        msg = 'Text file ({:s}): {:s}'.format(os.path.basename(self.truth_path), result)
        print(msg)
        print_obj(msg)