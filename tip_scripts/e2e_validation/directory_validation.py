import os
from pathlib import Path
from tip_scripts.e2e_validation.validation_base import ValidationBase

class DirectoryValidation(ValidationBase):

    def __init__(self, prefix):
        ValidationBase.__init__(self, prefix)
        self.validation_objects = []

    def validate_dir(self, print_obj):
        
        #if self.ready_to_validate:
        result_list = []

        for validation_obj in self.validation_objects:

            info = '\n' + str(validation_obj)
            print_obj(info)
            print(info)
            result = validation_obj.validate()
            msg = 'Validated: {}'.format(validation_obj.get_test_result_string())
            print_obj(msg)    
            print(msg)

            result_list.append(result)

        if result_list.count(None) > 0:
            self.test_passed = None
        elif result_list.count(False) > 0:
            self.test_passed = False
        else:
            self.test_passed = True

    def _create_validation_objects(self):
        print('DirectoryValidation._create_validation_objects(): Not overridden!')

    def print_results(self, print_obj):

        if len(self.validation_objects) > 0:
            print('')
            print_obj('')

            for vobj in self.validation_objects:
                vobj.print_result(print_obj)
        
            msg = 'Directory total: {:s}'.format(self.get_test_result_string())
            print(msg)
            print_obj(msg)
