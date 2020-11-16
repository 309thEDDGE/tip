from tip_scripts.e2e_validation.file_validation import FileValidation

class PqPqTranslated1553Validation(FileValidation):

    def __init__(self, truth_path, test_path, exec_path):
        prefix = 'PqPqTranslated1553Validation'
        FileValidation.__init__(self, prefix)
        self.ready_to_validate = self.set_1553_paths(truth_path, test_path, 'transl1553')
        self.exec_path = exec_path

    def validate(self):
        return self.do_directory_validation(self.exec_path)