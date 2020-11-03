import os
from tip_scripts.e2e_validation.file_validation import FileValidation
from tip_scripts.e2e_validation.yaml_compare import yaml_compare

class YmlYmlValidation(FileValidation):

    def __init__(self, truth_path, test_path):
        FileValidation.__init__(self, 'YmlYmlValidation')
        self.ready_to_validate = self.set_file_paths(truth_path, test_path)

    def validate(self):

        self.result = {}
        if not self.ready_to_validate:
            print('YmlYmlValidation.validate(): Not ready to validate.'.format(self.prefix))
            return self.test_passed

        yc = yaml_compare.YamlCompare(self.truth_path, self.test_path)
        self.result = yc.Compare()

        keys = list(self.result.keys())
        if len(keys) == 0:
            self.test_passed = None
        elif not 'total' in keys:
            self.test_passed = None
        else:
            self.test_passed = self.result['total']

        return self.test_passed

    def print_result(self, print_obj):
        # Custom override

        msg = '{:s} - {:s}: {:s}'.format(self.prefix, 
                                         os.path.basename(self.truth_path),
                                         self.get_test_result_string())
        print(msg)
        print_obj(msg)

        if self.test_passed is not None:
            for k in self.result.keys():
                if k != 'total':
                    msg = '- {:s}: {:s}'.format(k, self.get_test_result_string_from_input(self.result[k]))
                    print(msg)
                    print_obj(msg)
