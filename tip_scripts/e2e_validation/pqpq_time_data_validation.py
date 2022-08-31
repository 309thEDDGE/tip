from tip_scripts.e2e_validation.file_validation import FileValidation

class PqPqTimeDataValidation(FileValidation):

    def __init__(self, truth_path, test_path, pqcompare_exec_path):
        prefix = 'PqPqTimeDataValidation'
        FileValidation.__init__(self, prefix)
        self.pqcompare_exec_path = pqcompare_exec_path
        self.ready_to_validate = self.set_type_paths(truth_path, test_path, 'timef1')

    def validate(self, print_obj):

        info = '\n' + str(self)
        print_obj(info)
        print(info)
        result = self.do_file_validation(self.pqcompare_exec_path, output_success_string='PASS')
        msg = 'Validated: {}'.format(self.get_test_result_string())
        print_obj(msg)    
        print(msg)



