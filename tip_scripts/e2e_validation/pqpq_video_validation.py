from tip_scripts.e2e_validation.file_validation import FileValidation
#from tip_scripts.e2e_validation.ymlyml_validation import YmlYmlValidation
#from tip_scripts.e2e_validation.txttxt_validation import TxtTxtValidation

class PqPqVideoValidation(FileValidation):

    def __init__(self, truth_path, test_path, pqcompare_exec_path):
        prefix = 'PqPqVideoValidation'
        FileValidation.__init__(self, prefix)
        self.pqcompare_exec_path = pqcompare_exec_path
        self.ready_to_validate = self.set_1553_paths(truth_path, test_path, 'rawvideo')

    def validate(self):

       return self.do_directory_validation(self.pqcompare_exec_path)
