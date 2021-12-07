import os
from tip_scripts.e2e_validation.file_validation import FileValidation
from tip_scripts.e2e_validation.yaml_compare import yaml_compare

class YmlYmlValidation(FileValidation):

    def __init__(self, truth_path, test_path, exclude_func=None):
        FileValidation.__init__(self, 'YmlYmlValidation')
        self.exclude_func = exclude_func
        self.ready_to_validate = self.set_file_paths(truth_path, test_path)

    def validate(self):

        self.result = {}
        if not self.ready_to_validate:
            print('YmlYmlValidation.validate(): Not ready to validate.'.format(self.prefix))
            return self.test_passed

        yc = yaml_compare.YamlCompare(self.truth_path, self.test_path)
        self.result = yc.Compare(exclude_func=self.exclude_func)
        self.test_passed = self.result

        return self.test_passed
