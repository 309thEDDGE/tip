from tip_scripts.e2e_validation.file_validation import FileValidation

class PqPqRawARINC429Validation(FileValidation):

    def __init__(self, truth_path, test_path, pqcompare_exec_path):
        prefix = 'PqPqParsedARINC429F0Validation'
        FileValidation.__init__(self, prefix)
        self.pqcompare_exec_path = pqcompare_exec_path
        self.ready_to_validate = self.set_type_paths(truth_path, test_path, 'parsedarinc429f0')

    def validate(self):

        allowed_extensions = ['pq', 'parquet', 'Parquet']
        if not self.directory_has_files_with_extensions(self.truth_path, allowed_extensions):
           print('No files found with extensions: ', allowed_extensions)
           self.test_passed = True
           return True

        return self.do_directory_validation(self.pqcompare_exec_path)


