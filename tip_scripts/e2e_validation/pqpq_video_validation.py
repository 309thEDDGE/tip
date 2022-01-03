from tip_scripts.e2e_validation.file_validation import FileValidation
#from tip_scripts.e2e_validation.ymlyml_validation import YmlYmlValidation
#from tip_scripts.e2e_validation.txttxt_validation import TxtTxtValidation

class PqPqVideoValidation(FileValidation):

    def __init__(self, truth_path, test_path, pqcompare_exec_path):
        prefix = 'PqPqParsedVideoF0Validation'
        FileValidation.__init__(self, prefix)
        self.pqcompare_exec_path = pqcompare_exec_path
        self.ready_to_validate = self.set_type_paths(truth_path, test_path, 'parsedvideof0')

    def validate(self):

       allowed_extensions = ['pq', 'parquet', 'Parquet']
       if not self.directory_has_files_with_extensions(self.truth_path, allowed_extensions):
           print('No files found with extensions: ', allowed_extensions)
           self.test_passed = True
           return True

       return self.do_directory_validation(self.pqcompare_exec_path)
