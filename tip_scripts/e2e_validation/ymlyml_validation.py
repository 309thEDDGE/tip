import os
from tip_scripts.e2e_validation.file_validation import FileValidation
from tip_scripts.e2e_validation.yaml_compare import yaml_compare

class YmlYmlValidation(FileValidation):

    def __init__(self, truth_path, test_path, prefix):
        FileValidation.__init__(self, prefix)
        self.ready_to_validate = self.set_file_paths(truth_path, test_path)

    def validate(self):

        self.result = {}
        if not self.ready_to_validate:
            print('{:s} - YmlYmlValidation.validate(): Not ready to validate.'.format(self.prefix))
            return result

        yc = yaml_compare.YamlCompare(self.truth_path, self.test_path)
        #print('{:s} - YmlYmlValidation.validate(): Comparing {:s}, {:s}'.format(
        #    self.prefix, self.truth_path, self.test_path))
        self.result = yc.Compare()

        keys = list(self.result.keys())
        if len(keys) == 0:
            self.test_passed = None
        elif not 'total' in keys:
            self.test_passed = None
        else:
            self.test_passed = self.result['total']

        return self.test_passed

    def print_results(self, print_obj):

        msg = 'Yaml file ({:s}):'.format(os.path.basename(self.truth_path))
        print(msg)
        print_obj(msg)

        if self.test_passed is None:
            msg = 'Yaml total: NULL'
            print(msg)
            print_obj(msg)
        else:
            for k in self.result.keys():
                if k != 'total':
                    msg = 'key - {:s}: {:s}'.format(k, self.result[k])
                    print(msg)
                    print_obj(msg)
            msg = 'Yaml total: {:s}'.format(self.result['total'])
            print(msg)
            print_obj(msg)